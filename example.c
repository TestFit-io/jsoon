#include "json.h"

struct obj
{
	uint64_t n;
	struct {
		int32_t x;
		int32_t y;
	} points[8];
};

/* error checking omitted for brevity */
int obj_read(FILE *fp, struct obj *obj)
{
	json_t json;
	json_obj_t list, elem;
	json_read_begin(&json, fp);
	json_read_uint64(&json, "n", &obj->n);
	json_read_array_begin(&json, "points", &list);
	for (uint64_t i = 0; i < obj->n; ++i) {
		json_read_object_begin(&json, "point", &elem);
		json_read_int32(&json, "x", &obj->points[i].x);
		json_read_int32(&json, "y", &obj->points[i].y);
		json_read_object_end(&json);
	}
	json_read_array_end(&json);
	json_read_end(&json);
	return 0;
}

int obj_write(FILE *fp, const struct obj *obj)
{
	json_t json;
	json_obj_t list, elem;
	json_write_begin(&json, fp);
	json_write_uint64(&json, "n", obj->n);
	json_write_array_begin(&json, "points", &list);
	for (uint64_t i = 0; i < obj->n; ++i) {
		json_write_object_begin(&json, "point", &elem);
		json_write_int32(&json, "x", obj->points[i].x);
		json_write_int32(&json, "y", obj->points[i].y);
		json_write_object_end(&json);
	}
	json_write_array_end(&json);
	json_write_end(&json);
	return 0;
}

int main(void)
{
	struct obj obj;
	FILE *in, *out;

	in = fopen("in.json", "rb");
	if (!in) {
		printf("failed to open infile\n");
		return 1;
	}
	obj_read(in, &obj);
	fclose(in);

	out = fopen("out.json", "w");
	if (!out) {
		printf("failed to open outfile\n");
		return 1;
	}
	obj_write(out, &obj);
	fclose(out);

	return 0;
}
