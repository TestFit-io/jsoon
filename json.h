#ifndef JSON_H
#define JSON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define JSON_PRETTY_PRINT


typedef struct json_obj
{
	size_t n;
	bool is_array;
	struct json_obj *prev;
} json_obj_t;

typedef struct json
{
	FILE *fp;
	size_t indent;
	json_obj_t root;
	json_obj_t *cur;
} json_t;

void json_write_begin(json_t *json, FILE *fp);
void json_write_end(json_t *json);

void json_write_object_begin(json_t *json, const char *label, json_obj_t *obj);
void json_write_object_end(json_t *json);
void json_write_array_begin(json_t *json, const char *label, json_obj_t *obj);
void json_write_array_end(json_t *json);
void json_write_bool(json_t *json, const char *label, bool val);
void json_write_int32(json_t *json, const char *label, int32_t val);
void json_write_uint32(json_t *json, const char *label, uint32_t val);
void json_write_float(json_t *json, const char *label, float val);
void json_write_int64(json_t *json, const char *label, int64_t val);
void json_write_uint64(json_t *json, const char *label, uint64_t val);
void json_write_double(json_t *json, const char *label, double val);
void json_write_char(json_t *json, const char *label, char val);
void json_write_str(json_t *json, const char *label, const char *val);
void json_write_strn(json_t *json, const char *label, const char *val, size_t n);


bool json_read_begin(json_t *json, FILE *fp);
bool json_read_end(json_t *json);

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

#endif // TESTFIT_JSON_H
