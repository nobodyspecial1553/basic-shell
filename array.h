#ifndef ARRAY_H_
#define ARRAY_H_

#include "types.h"

#define DYNAMIC_ARRAY_DEFAULT_CAPACITY GIGABYTE

struct Array {
	byte *data;
	i64 length;
};

struct Dynamic_Array { /* Fixed-size dynamic array, no resize */
	byte *data;
	i64 length;
	i64 capacity;
};

static void dyanmic_array_init(struct Dynamic_Array *dynamic_array, u64 capacity) {
	dynamic_array->length = 0;
	dynamic_array->capacity = capacity;
	dynamic_array->data = mmap(nil, dynamic_array->capacity, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
}

#define dynamic_array_push(dynamic_array, data_ptr, count)\
	do {\
		dynamic_array_push_bytes(dynamic_array, (byte*)data_ptr, sizeof(*data_ptr) * count);\
	} while(0)

#define dynamic_array_pop(dynamic_array, data_size, count)\
	do {\
		dynamic_array_pop_bytes(dynamic_array, data_size * count);\
	} while(0)

static void dynamic_array_push_bytes(struct Dynamic_Array *dynamic_array, byte *data, i64 data_length) {
	byte *append_ptr;
	i64 i;

	if (dynamic_array->data == nil) {
		dyanmic_array_init(dynamic_array, DYNAMIC_ARRAY_DEFAULT_CAPACITY);
	}

	append_ptr = &dynamic_array->data[dynamic_array->length];
	dynamic_array->length += data_length;

	for (i = 0; i < data_length; i++) {
		append_ptr[i] = data[i];
	}
}

static void dynamic_array_pop_bytes(struct Dynamic_Array *dynamic_array, i64 bytes) {
	if (dynamic_array->data == nil || bytes == 0) {
		return;
	}

	dynamic_array->length -= bytes;
	dynamic_array->length = (dynamic_array->length < 0 ? 0 : dynamic_array->length);
}

static void dynamic_array_clear(struct Dynamic_Array *dynamic_array) {
	if (dynamic_array->data == nil) {
		return;
	}

	dynamic_array->length = 0;
}

static struct Array array_from_dynamic_array(struct Dynamic_Array dynamic_array) {
	struct Array array;

	array.data = dynamic_array.data;
	array.length = dynamic_array.length;

	return array;
}

#endif /* ARRAY_H_ */
