#include "json.h"
#include <string.h>

struct obj
{
	uint64_t n;
	struct {
		int32_t x;
		int32_t y;
	} points[8];
};

static const char *g_str = "{\"n\": 4,\"points\": [{\"x\": 0,\"y\": 0},{\"x\": 10,\"y\": 0},{\"x\": 10,\"y\": 10},{\"x\": 0,\"y\": 10}]}";

/* error checking omitted for brevity */
int obj_read(const char *str, size_t len, struct obj *obj)
{
	json_t json;
	json_mem_t mem = { .buf = (char*)str, .len = len };
	json_obj_t root, list, elem;
	json_init_mem(&json, &mem);
	json_read_object_begin(&json, "root", &root);
	json_read_uint64(&json, "n", &obj->n);
	json_read_array_begin(&json, "points", &list);
	for (uint64_t i = 0; i < obj->n; ++i) {
		json_read_object_begin(&json, "point", &elem);
		json_read_int32(&json, "x", &obj->points[i].x);
		json_read_int32(&json, "y", &obj->points[i].y);
		json_read_object_end(&json);
	}
	json_read_array_end(&json);
	json_read_object_end(&json);
	return 0;
}

int obj_write(FILE *fp, const struct obj *obj)
{
	json_t json;
	json_obj_t root, list, elem;
	json_init_file(&json, fp);
	json_write_object_begin(&json, "root", &root);
	json_write_uint64(&json, "n", obj->n);
	json_write_array_begin(&json, "points", &list);
	for (uint64_t i = 0; i < obj->n; ++i) {
		json_write_object_begin(&json, "point", &elem);
		json_write_int32(&json, "x", obj->points[i].x);
		json_write_int32(&json, "y", obj->points[i].y);
		json_write_object_end(&json);
	}
	json_write_array_end(&json);
	json_write_object_end(&json);
	return 0;
}

int main(void)
{
	struct obj obj;

	obj_read(g_str, strlen(g_str), &obj);
	obj_write(stdout, &obj);
	fputc('\n', stdout);

	return 0;
}
