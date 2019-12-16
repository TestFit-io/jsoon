#include <ctype.h>
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "json.h"

/* writing */

static
void json__init(json_t *json, FILE *fp)
{
	json->fp = fp;
	json->indent = 0;
	json->root.n = 0;
	json->root.is_array = true;
	json->root.prev = NULL;
	json->cur = &json->root;
}

void json_write_begin(json_t *json, FILE *fp)
{
	json__init(json, fp);
	json_write_object_begin(json, "root", &json->root);
}

void json_write_end(json_t *json)
{
	assert(json->indent == 1);
	json_write_object_end(json);
}

static
void json__write_newline(json_t *json)
{
#ifdef JSON_PRETTY_PRINT
	fputc('\n', json->fp);
#endif
}

static
void json__write_member_separator(json_t *json)
{
	if (json->cur->n > 0)
		fputc(',', json->fp);
	json__write_newline(json);
	++json->cur->n;
}

static
void json__write_indent(json_t *json)
{
#ifdef JSON_PRETTY_PRINT
	for (size_t i = 0; i < json->indent; ++i)
		fputc(' ', json->fp);
#endif
}

static
void json__write_str(json_t *json, const char *str)
{
	fputc('"', json->fp);
	fputs(str, json->fp);
	fputc('"', json->fp);
}

static
void json__write_strn(json_t *json, const char *buf, size_t n)
{
	fputc('"', json->fp);
	fwrite(buf, n, 1, json->fp);
	fputc('"', json->fp);
}

static
void json__write_label(json_t *json, const char *label)
{
	json__write_member_separator(json);
	json__write_indent(json);
	if (!json->cur->is_array) {
		json__write_str(json, label);
#ifdef JSON_PRETTY_PRINT
		fputs(": ", json->fp);
#else
		fputs(":", json->fp);
#endif
	}
}

static
void json__push_obj(json_t *json, json_obj_t *obj, bool is_array)
{
	++json->indent;

	obj->n = 0;
	obj->is_array = is_array;
	obj->prev = json->cur;

	json->cur = obj;
}

static
void json__write_object_begin(json_t *json, const char *label,
                              bool is_array, json_obj_t *obj)
{
	const char open[] = { '{', '[' };

	json__write_label(json, label);
	fputc(open[is_array], json->fp);
	json__push_obj(json, obj, is_array);
}

static
void json__write_object_end(json_t *json, bool is_array)
{
	const char close[] = { '}', ']' };

	assert(json->indent > 0);
	assert(json->cur->is_array == is_array);

	--json->indent;

	if (json->cur->n > 0) {
		json__write_newline(json);
		json__write_indent(json);
	}

	fputc(close[is_array], json->fp);

	json->cur = json->cur->prev;
}

void json_write_object_begin(json_t *json, const char *label, json_obj_t *obj)
{
	json__write_object_begin(json, label, false, obj);
}

void json_write_object_end(json_t *json)
{
	json__write_object_end(json, false);
}

void json_write_array_begin(json_t *json, const char *label, json_obj_t *obj)
{
	json__write_object_begin(json, label, true, obj);
}

void json_write_array_end(json_t *json)
{
	json__write_object_end(json, true);
}

void json_write_bool(json_t *json, const char *label, bool val)
{
	json_write_str(json, label, val ? "true" : "false");
}

void json_write_int32(json_t *json, const char *label, int32_t val)
{
	json__write_label(json, label);
	fprintf(json->fp, "%" PRId32, val);
}

void json_write_uint32(json_t *json, const char *label, uint32_t val)
{
	json__write_label(json, label);
	fprintf(json->fp, "%" PRIu32, val);
}

void json_write_float(json_t *json, const char *label, float val)
{
	json__write_label(json, label);
	fprintf(json->fp, "%f", val);
}

void json_write_int64(json_t *json, const char *label, int64_t val)
{
	json__write_label(json, label);
	fprintf(json->fp, "%" PRId64, val);
}

void json_write_uint64(json_t *json, const char *label, uint64_t val)
{
	json__write_label(json, label);
	fprintf(json->fp, "%" PRIu64, val);
}

void json_write_double(json_t *json, const char *label, double val)
{
	json__write_label(json, label);
	fprintf(json->fp, "%f", val);
}

void json_write_char(json_t *json, const char *label, char val)
{
	const char str[2] = { val, 0 };
	json_write_str(json, label, str);
}

void json_write_str(json_t *json, const char *label, const char *val)
{
	json__write_label(json, label);
	json__write_str(json, val);
}

void json_write_strn(json_t *json, const char *label, const char *val, size_t n)
{
	json__write_label(json, label);
	json__write_strn(json, val, n);
}


/* reading */

static
char json__read_past_whitespace(FILE *fp)
{
#ifdef JSON_PRETTY_PRINT
	int c;
	while ((c = fgetc(fp)) != EOF && isspace(c))
		;
	return c;
#else
	return fgetc(fp);
#endif
}

static
void json__skip_whitespace(FILE *fp)
{
#ifdef JSON_PRETTY_PRINT
	int c;
	while ((c = fgetc(fp)) != EOF && isspace(c))
		;
	ungetc(c, fp);
#endif
}

static
bool json__read_label_characters(FILE *fp, const char *label)
{
	const char *p = label;
	while (*p != 0) {
		const char c = fgetc(fp);
		if (*p != c)
			return false;
		++p;
	}

	return true;
}

static
bool json__read_label(json_t *json, const char *label)
{
	if (json->cur->n > 0 && json__read_past_whitespace(json->fp) != ',')
		return false;

	++json->cur->n;

	if (json->cur->is_array)
		return true;

	return json__read_past_whitespace(json->fp) == '"'
	    && json__read_label_characters(json->fp, label)
	    && json__read_past_whitespace(json->fp) == '"'
	    && json__read_past_whitespace(json->fp) == ':';
}


static
bool json__read_str(FILE *fp, char *str, size_t max)
{
	char *end = str + max;
	char *p = str;
	while ((*p = fgetc(fp)) != EOF && *p != '"' && p != end)
		++p;
	ungetc(*p, fp);
	*p = 0;
	return p != end;
}

static
bool json__read_digits(FILE *fp, char *str, size_t max)
{
	char *end = str + max;
	char *p = str;
	while (   (*p = fgetc(fp)) != EOF
	       && (isdigit(*p) || *p == '-' || *p == '.')
	       && p != end)
		++p;
	ungetc(*p, fp);
	if (p > str && p < end) {
		*p = 0;
		return true;
	}
	return false;
}

/* 20 digits can hold 2^64-1, so 64 should be plenty */
static
bool json__read_num(json_t *json, const char *label, char buf[64])
{
	if (!json__read_label(json, label))
		return false;

	json__skip_whitespace(json->fp);

	return json__read_digits(json->fp, buf, 64);
}

bool json_read_begin(json_t *json, FILE *fp)
{
	json__init(json, fp);
	return json_read_object_begin(json, "root", &json->root);
}

bool json_read_end(json_t *json)
{
	assert(json->indent == 1);
	return json_read_object_end(json);
}

bool json_read_member_label(json_t *json, const char *label)
{
	return json__read_label(json, label);
}

bool json_read_object_begin(json_t *json, const char *label, json_obj_t *obj)
{
	if (!json__read_label(json, label))
		return false;

	json__push_obj(json, obj, false);

	return json__read_past_whitespace(json->fp) == '{';
}

bool json_read_object_end(json_t *json)
{
	assert(json->indent > 0);
	json->cur = json->cur->prev;
	--json->indent;
	return json__read_past_whitespace(json->fp) == '}';
}

bool json_read_array_begin(json_t *json, const char *label, json_obj_t *obj)
{
	if (!json__read_label(json, label))
		return false;

	json__push_obj(json, obj, true);

	return json__read_past_whitespace(json->fp) == '[';
}

bool json_read_array_end(json_t *json)
{
	json->cur = json->cur->prev;
	--json->indent;
	return json__read_past_whitespace(json->fp) == ']';
}

bool json_read_bool(json_t *json, const char *label, bool *val)
{
	char str[6];
	if (!json_read_str(json, label, str, 6)) {
		return false;
	} else if (strcmp(str, "true") == 0) {
		*val = true;
		return true;
	} else if (strcmp(str, "false") == 0) {
		*val = false;
		return true;
	} else {
		return false;
	}
}

bool json_read_int8(json_t *json, const char *label, int8_t *val)
{
	int32_t val32;
	if (!json_read_int32(json, label, &val32) || val32 < INT8_MIN || val32 > INT8_MAX)
		return false;

	*val = (int8_t)val32;
	return true;
}

bool json_read_uint8(json_t *json, const char *label, uint8_t *val)
{
	uint32_t val32;
	if (!json_read_uint32(json, label, &val32) || (val32 & ~0xFF))
		return false;

	*val = (uint8_t)val32;
	return true;
}

bool json_read_int16(json_t *json, const char *label, int16_t *val)
{
	int32_t val32;
	if (!json_read_int32(json, label, &val32) || val32 < INT16_MIN || val32 > INT16_MAX)
		return false;

	*val = (int16_t)val32;
	return true;
}

bool json_read_uint16(json_t *json, const char *label, uint16_t *val)
{
	uint32_t val32;
	if (!json_read_uint32(json, label, &val32) || (val32 & ~0xFFFF))
		return false;

	*val = (uint16_t)val32;
	return true;
}

bool json_read_int32(json_t *json, const char *label, int32_t *val)
{
	char str[64];
	if (json__read_num(json, label, str)) {
		*val = strtol(str, NULL, 10);
		return true;
	}
	return false;
}

bool json_read_uint32(json_t *json, const char *label, uint32_t *val)
{
	char str[64];
	if (json__read_num(json, label, str)) {
		*val = strtoul(str, NULL, 10);
		return true;
	}
	return false;
}

bool json_read_float(json_t *json, const char *label, float *val)
{
	char str[64];
	if (json__read_num(json, label, str)) {
		*val = strtof(str, NULL);
		return true;
	}
	return false;
}

bool json_read_int64(json_t *json, const char *label, int64_t *val)
{
	char str[64];
	if (json__read_num(json, label, str)) {
		*val = strtoll(str, NULL, 10);
		return true;
	}
	return false;
}

bool json_read_uint64(json_t *json, const char *label, uint64_t *val)
{
	char str[64];
	if (json__read_num(json, label, str)) {
		*val = strtoull(str, NULL, 10);
		return true;
	}
	return false;
}

bool json_read_double(json_t *json, const char *label, double *val)
{
	char str[64];
	if (json__read_num(json, label, str)) {
		*val = strtod(str, NULL);
		return true;
	}
	return false;
}

bool json_read_char(json_t *json, const char *label, char *val)
{
	char str[2];
	return json_read_str(json, label, str, 2)
	    && str[1] == 0
	    && (*val = str[0]) != 0;
}

bool json_read_str(json_t *json, const char *label, char *val, size_t max)
{
	return json__read_label(json, label)
	    && json__read_past_whitespace(json->fp) == '"'
	    && json__read_str(json->fp, val, max)
	    && fgetc(json->fp) == '"';
}

bool json_read_strn(json_t *json, const char *label, char *val, size_t n)
{
	return json__read_label(json, label)
	    && json__read_past_whitespace(json->fp) == '"'
	    && fread(val, 1, n, json->fp) == n
	    && fgetc(json->fp) == '"';
}
