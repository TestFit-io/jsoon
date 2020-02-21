#include <ctype.h>
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "json.h"

/* initialization */

#define json__min(a, b) ((a) < (b) ? (a) : (b))

static
int json__mem_fgetc(void *user)
{
	json_mem_t *mem = user;
	return mem->pos < mem->len ? mem->buf[mem->pos++] : EOF;
}

static
int json__mem_ungetc(int c, void *user)
{
	json_mem_t *mem = user;
	return mem->pos > 0 && mem->buf[mem->pos-1] == c ? (mem->pos--, c) : EOF;
}

static
size_t json__mem_fread(void *ptr, size_t size, size_t nmemb, void *user)
{
	assert(size == 1);
	json_mem_t *mem = user;
	const size_t len = json__min(nmemb, mem->len - mem->pos);
	memcpy(ptr, &mem->buf[mem->pos], len);
	mem->pos += len;
	return len;
}

static
size_t json__mem_fwrite(const void *ptr, size_t size, size_t nmemb, void *user)
{
	assert(size == 1);
	json_mem_t *mem = user;
	const size_t len = json__min(nmemb, mem->len - mem->pos);
	memcpy(&mem->buf[mem->pos], ptr, len);
	mem->pos += len;
	return len;
}

static
int json__mem_fputc(int c, void *user)
{
	json_mem_t *mem = user;
	return mem->pos < mem->len ? (mem->buf[mem->pos++] = c) : EOF;
}

static
int json__file_fgetc(void *user)
{
	return fgetc(user);
}

static
int json__file_ungetc(int c, void *user)
{
	return ungetc(c, user);
}

static
size_t json__file_fread(void *ptr, size_t size, size_t nmemb, void *user)
{
	return fread(ptr, size, nmemb, user);
}

static
size_t json__file_fwrite(const void *ptr, size_t size, size_t nmemb, void *user)
{
	return fwrite(ptr, size, nmemb, user);
}

static
int json__file_fputc(int c, void *user)
{
	return fputc(c, user);
}

const json_io_t g_json_io_mem = {
	.fgetc  = json__mem_fgetc,
	.ungetc = json__mem_ungetc,
	.fread  = json__mem_fread,
	.fwrite = json__mem_fwrite,
	.fputc  = json__mem_fputc,
};

const json_io_t g_json_io_file = {
	.fgetc  = json__file_fgetc,
	.ungetc = json__file_ungetc,
	.fread  = json__file_fread,
	.fwrite = json__file_fwrite,
	.fputc  = json__file_fputc,
};

void json_init(json_t *json, json_io_t io, void *user)
{
	json->user = user;
	json->io = io;
	json->indent = 0;
	json->root.n = 0;
	json->root.is_array = true;
	json->root.prev = NULL;
	json->cur = &json->root;
}

void json_init_file(json_t *json, FILE *fp)
{
	json_init(json, g_json_io_file, fp);
}

void json_init_mem(json_t *json, json_mem_t *mem)
{
	json_init(json, g_json_io_mem, mem);
}

/* writing */

static
bool json__write_newline(json_t *json)
{
#if JSON_PRETTY_PRINT
	return json->io.fputc('\n', json->user) != EOF;
#endif
}

static
bool json__write_member_separator(json_t *json)
{
	if (json->cur->n > 0 && json->io.fputc(',', json->user) == EOF)
		return false;
	if (json->cur != &json->root && !json__write_newline(json))
		return false;
	++json->cur->n;
	return true;
}

static
bool json__write_indent(json_t *json)
{
#if JSON_PRETTY_PRINT
	for (size_t i = 0; i < json->indent; ++i)
		for (size_t j = 0; j < JSON_INDENT_SIZE; ++j)
			if (json->io.fputc(' ', json->user) == EOF)
				return false;
#endif
	return true;
}

static
bool json__write_strn(json_t *json, const char *buf, size_t n)
{
	return json->io.fputc('"', json->user) != EOF
	    && json->io.fwrite(buf, 1, n, json->user) == n
	    && json->io.fputc('"', json->user) != EOF;
}

static
bool json__write_colon(json_t *json)
{
#if JSON_PRETTY_PRINT
	return json->io.fwrite(": ", 1, 2, json->user);
#else
	return json->io.fputc(':', json->user);
#endif
}

static
bool json__write_label(json_t *json, const char *label)
{
	return json__write_member_separator(json)
	    && json__write_indent(json)
	    && (   json->cur->is_array
	        || (   json__write_strn(json, label, strlen(label))
	            && json__write_colon(json)));
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
bool json__write_object_begin(json_t *json, const char *label,
                              bool is_array, json_obj_t *obj)
{
	const char open[] = { '{', '[' };

	if (   !json__write_label(json, label)
	    || json->io.fputc(open[is_array], json->user) == EOF)
		return false;

	json__push_obj(json, obj, is_array);
	return true;
}

static
bool json__write_object_end(json_t *json, bool is_array)
{
	const char close[] = { '}', ']' };

	assert(json->indent > 0);
	assert(json->cur->is_array == is_array);

	--json->indent;

	if (   json->cur->n > 0
	    && (   !json__write_newline(json)
	        || !json__write_indent(json)))
		return false;

	if (json->io.fputc(close[is_array], json->user) == EOF)
		return false;

	json->cur = json->cur->prev;
	return true;
}

bool json_write_object_begin(json_t *json, const char *label, json_obj_t *obj)
{
	return json__write_object_begin(json, label, false, obj);
}

bool json_write_object_end(json_t *json)
{
	return json__write_object_end(json, false);
}

bool json_write_array_begin(json_t *json, const char *label, json_obj_t *obj)
{
	return json__write_object_begin(json, label, true, obj);
}

bool json_write_array_end(json_t *json)
{
	return json__write_object_end(json, true);
}

bool json_write_bool(json_t *json, const char *label, bool val)
{
	return json__write_label(json, label)
	    &&   val
	       ? json->io.fwrite("true", 1, 4, json->user) == 4
	       : json->io.fwrite("false", 1, 5, json->user) == 5;
}

bool json_write_int32(json_t *json, const char *label, int32_t val)
{
	char str[64];
	int len = sprintf(str, "%" PRId32, val);
	assert(len < 64);
	return json__write_label(json, label)
	    && json->io.fwrite(str, 1, len, json->user) == len;
}

bool json_write_uint32(json_t *json, const char *label, uint32_t val)
{
	char str[64];
	int len = sprintf(str, "%" PRIu32, val);
	assert(len < 64);
	return json__write_label(json, label)
	    && json->io.fwrite(str, 1, len, json->user) == len;
}

bool json_write_float(json_t *json, const char *label, float val)
{
	char str[64];
	int len = sprintf(str, "%f", val);
	assert(len < 64);
	return json__write_label(json, label)
	    && json->io.fwrite(str, 1, len, json->user) == len;
}

bool json_write_int64(json_t *json, const char *label, int64_t val)
{
	char str[64];
	int len = sprintf(str, "%" PRId64, val);
	assert(len < 64);
	return json__write_label(json, label)
	    && json->io.fwrite(str, 1, len, json->user) == len;
}

bool json_write_uint64(json_t *json, const char *label, uint64_t val)
{
	char str[64];
	int len = sprintf(str, "%" PRIu64, val);
	assert(len < 64);
	return json__write_label(json, label)
	    && json->io.fwrite(str, 1, len, json->user) == len;
}

bool json_write_double(json_t *json, const char *label, double val)
{
	char str[64];
	int len = sprintf(str, "%f", val);
	assert(len < 64);
	return json__write_label(json, label)
	    && json->io.fwrite(str, 1, len, json->user) == len;
}

bool json_write_char(json_t *json, const char *label, char val)
{
	const char str[2] = { val, 0 };
	return json_write_str(json, label, str);
}

bool json_write_str(json_t *json, const char *label, const char *val)
{
	return json__write_label(json, label)
	    && json__write_strn(json, val, strlen(val));
}

bool json_write_strn(json_t *json, const char *label, const char *val, size_t n)
{
	return json__write_label(json, label)
	    && json__write_strn(json, val, n);
}


/* reading */

static
char json__read_past_whitespace(json_t *json)
{
#if JSON_PRETTY_PRINT
	int c;
	while ((c = json->io.fgetc(json->user)) != EOF && isspace(c))
		;
	return c;
#else
	return json->io.fgetc(json->user);
#endif
}

static
void json__skip_whitespace(json_t *json)
{
#if JSON_PRETTY_PRINT
	int c;
	while ((c = json->io.fgetc(json->user)) != EOF && isspace(c))
		;
	json->io.ungetc(c, json->user);
#endif
}

static
bool json__read_exact(json_t *json, const char *label)
{
	const char *p = label;
	while (*p != 0) {
		const char c = json->io.fgetc(json->user);
		if (*p != c)
			return false;
		++p;
	}

	return true;
}

static
bool json__read_label(json_t *json, const char *label)
{
	if (json->cur->n > 0 && json__read_past_whitespace(json) != ',')
		return false;

	++json->cur->n;

	if (json->cur->is_array)
		return true;

	return json__read_past_whitespace(json) == '"'
	    && json__read_exact(json, label)
	    && json__read_past_whitespace(json) == '"'
	    && json__read_past_whitespace(json) == ':';
}


static
bool json__read_str(json_t *json, char *str, size_t max)
{
	char *end = str + max;
	char *p = str;
	while ((*p = json->io.fgetc(json->user)) != EOF && *p != '"' && p != end)
		++p;
	json->io.ungetc(*p, json->user);
	*p = 0;
	return p != end;
}

static
bool json__read_digits(json_t *json, char *str, size_t max)
{
	char *end = str + max;
	char *p = str;
	while (   (*p = json->io.fgetc(json->user)) != EOF
	       && (isdigit(*p) || *p == '-' || *p == '.')
	       && p != end)
		++p;
	json->io.ungetc(*p, json->user);
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

	json__skip_whitespace(json);

	return json__read_digits(json, buf, 64);
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

	return json__read_past_whitespace(json) == '{';
}

bool json_read_object_end(json_t *json)
{
	assert(json->indent > 0);
	json->cur = json->cur->prev;
	--json->indent;
	return json__read_past_whitespace(json) == '}';
}

bool json_read_array_begin(json_t *json, const char *label, json_obj_t *obj)
{
	if (!json__read_label(json, label))
		return false;

	json__push_obj(json, obj, true);

	return json__read_past_whitespace(json) == '[';
}

bool json_read_array_end(json_t *json)
{
	json->cur = json->cur->prev;
	--json->indent;
	return json__read_past_whitespace(json) == ']';
}

bool json_read_bool(json_t *json, const char *label, bool *val)
{
	if (!json__read_label(json, label))
		return false;

	switch (json__read_past_whitespace(json)) {
	case 't':
	  return json__read_exact(json, "rue");
	case 'f':
	  return json__read_exact(json, "alse");
	default:
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
	    && json__read_past_whitespace(json) == '"'
	    && json__read_str(json, val, max)
	    && json->io.fgetc(json->user) == '"';
}

bool json_read_strn(json_t *json, const char *label, char *val, size_t n)
{
	return json__read_label(json, label)
	    && json__read_past_whitespace(json) == '"'
	    && json->io.fread(val, 1, n, json->user) == n
	    && json->io.fgetc(json->user) == '"';
}
