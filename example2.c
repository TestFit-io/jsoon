#include "json.h"
#include <string.h>
#include <stdlib.h>

struct point
{
	int32_t x;
	int32_t y;
};

struct obj
{
	uint64_t n;
	struct point points[8];
};

static const char *g_str = "{\"n\": 4,\"points\": [{\"x\": 0,\"y\": 0},{\"x\": 10,\"y\": 0},{\"x\": 10,\"y\": 10},{\"x\": 0,\"y\": 10}]}";

static
bool err(const char *str)
{
	fprintf(stderr, "err @ %s\n", str);
	return false;
}

#define CHECK(func, ...) do { if (!func(__VA_ARGS__)) return err(#func); } while (0)

bool obj_read_point(json_t *json, struct point *p, const char *name)
{
	json_obj_t elem;
	CHECK(json_read_object_begin, json, name, &elem);
	CHECK(json_read_int32, json, "x", &p->x);
	CHECK(json_read_int32, json, "y", &p->y);
	CHECK(json_read_object_end, json);
	return true;
}

bool obj_read(const char *str, size_t len, struct obj *obj)
{
	json_t json;
	json_mem_t mem = { .buf = (char*)str, .len = len };
	json_obj_t root, list;
	json_init_mem(&json, &mem);
	CHECK(json_read_object_begin, &json, "root", &root);
	CHECK(json_read_uint64, &json, "n", &obj->n);
	CHECK(json_read_array_begin, &json, "points", &list);
	for (uint64_t i = 0; i < obj->n; ++i)
		CHECK(obj_read_point, &json, &obj->points[i], "point");
	CHECK(json_read_array_end, &json);
	CHECK(json_read_object_end, &json);
	return true;
}

bool obj_write_point(json_t *json, const struct point *p, const char *name)
{
	json_obj_t elem;
	CHECK(json_write_object_begin, json, name, &elem);
	CHECK(json_write_int32, json, "x", p->x);
	CHECK(json_write_int32, json, "y", p->y);
	CHECK(json_write_object_end, json);
	return true;
}

bool obj_write(char *buf, size_t len, const struct obj *obj)
{
	json_t json;
	json_obj_t root, list;
	json_mem_t mem = { .buf = buf, .len = len };
	json_init_mem(&json, &mem);
	CHECK(json_write_object_begin, &json, "root", &root);
	CHECK(json_write_uint64, &json, "n", obj->n);
	CHECK(json_write_array_begin, &json, "points", &list);
	for (uint64_t i = 0; i < obj->n; ++i)
		CHECK(obj_write_point, &json, &obj->points[i], "point");
	CHECK(json_write_array_end, &json);
	CHECK(json_write_object_end, &json);
	mem.buf[mem.pos] = 0;
	return true;
}

int main(void)
{
	struct obj obj;
	size_t len = 64;
	char *buf = malloc(len);
	bool success = false;

	if (!obj_read(g_str, strlen(g_str), &obj)) {
		err("obj_read");
		goto out;
	}

	while (!(success = obj_write(buf, len, &obj)) && len < 1024) {
		err("obj_write\n");
		buf = realloc(buf, len *= 2);
	}

	if (success)
		printf("%s\n", buf);

out:
	return success ? 0 : 1;
}
