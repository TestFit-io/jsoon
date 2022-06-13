#ifndef JSON_H
#define JSON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifndef JSON_PRETTY_PRINT
#define JSON_PRETTY_PRINT 1
#endif

#ifndef JSON_INDENT_SIZE
#define JSON_INDENT_SIZE 1
#endif

typedef struct json_mem
{
	char *buf; /* effectively const for read calls */
	size_t pos, len;
} json_mem_t;

typedef struct json_io
{
	int(*fgetc)(void *user);
	int(*ungetc)(int c, void *user);
	size_t(*fread)(void *ptr, size_t size, size_t nmemb, void *user);
	size_t(*fwrite)(const void *ptr, size_t size, size_t nmemb, void *user);
	int(*fputc)(int c, void *user);
} json_io_t;

typedef struct json_obj
{
	size_t n;
	bool is_array;
	struct json_obj *prev;
} json_obj_t;

typedef struct json
{
	void *user;
	json_io_t io;
	size_t indent;
	size_t line;
	json_obj_t root;
	json_obj_t *cur;
} json_t;

extern const json_io_t g_json_io_mem;
extern const json_io_t g_json_io_file;

void json_init(json_t *json, json_io_t io, void *user);
void json_init_file(json_t *json, FILE *fp);
void json_init_mem(json_t *json, json_mem_t *mem);

bool json_write_object_begin(json_t *json, const char *label, json_obj_t *obj);
bool json_write_object_end(json_t *json);
bool json_write_array_begin(json_t *json, const char *label, json_obj_t *obj);
bool json_write_array_end(json_t *json);
bool json_write_bool(json_t *json, const char *label, bool val);
bool json_write_int32(json_t *json, const char *label, int32_t val);
bool json_write_uint32(json_t *json, const char *label, uint32_t val);
bool json_write_float(json_t *json, const char *label, float val);
bool json_write_int64(json_t *json, const char *label, int64_t val);
bool json_write_uint64(json_t *json, const char *label, uint64_t val);
bool json_write_double(json_t *json, const char *label, double val);
bool json_write_char(json_t *json, const char *label, char val);
bool json_write_str(json_t *json, const char *label, const char *val);
bool json_write_strn(json_t *json, const char *label, const char *val, size_t n);
bool json_write_str_unescaped(json_t *json, const char *label, const char *val);

bool json_read_member_label(json_t *json, const char *label);
bool json_read_object_begin(json_t *json, const char *label, json_obj_t *obj);
bool json_read_object_end(json_t *json);
bool json_read_array_begin(json_t *json, const char *label, json_obj_t *obj);
bool json_read_array_end(json_t *json);
bool json_read_bool(json_t *json, const char *label, bool *val);
bool json_read_int8(json_t *json, const char *label, int8_t *val);
bool json_read_uint8(json_t *json, const char *label, uint8_t *val);
bool json_read_int16(json_t *json, const char *label, int16_t *val);
bool json_read_uint16(json_t *json, const char *label, uint16_t *val);
bool json_read_int32(json_t *json, const char *label, int32_t *val);
bool json_read_uint32(json_t *json, const char *label, uint32_t *val);
bool json_read_float(json_t *json, const char *label, float *val);
bool json_read_int64(json_t *json, const char *label, int64_t *val);
bool json_read_uint64(json_t *json, const char *label, uint64_t *val);
bool json_read_double(json_t *json, const char *label, double *val);
bool json_read_char(json_t *json, const char *label, char *val);
bool json_read_str(json_t *json, const char *label, char *val, size_t max);
bool json_read_strn(json_t *json, const char *label, char *val, size_t n);
bool json_read_str_part(json_t *json, const char *label, char *val, size_t max, size_t *len, bool *more);

bool json_peek_array_end(json_t *json);
bool json_peek_data_end(json_t *json);

#endif // JSON_H
