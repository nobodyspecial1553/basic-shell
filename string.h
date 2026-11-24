#ifndef STRING_H_
#define STRING_H_

#include "types.h"
#include "array.h"

struct String {
	char *data;
	i64 length; /* Does not account for Unicode */
};

#define string_static_init(s) { s, sizeof(s) - 1 }

/*
	 A length of zero will translate to 'full length'
*/
static struct String string_slice(struct String base_string, i64 offset, i64 length) {
	struct String string = { 0 };
	i64 out_of_bounds = 0;

	offset = (offset < 0 ? 0 : offset);
	length = (length <= 0 ? base_string.length : length);
	out_of_bounds = base_string.length - (offset + length);
	if (out_of_bounds < 0) {
		length += out_of_bounds; /* Clamps the length */
	}

	string.data = &base_string.data[offset];
	string.length = length;

	return string;
}

/*
	 A length of zero will translate to 'full length'
*/
static struct String string_from_array(struct Array array, i64 offset, i64 length) {
	struct String string_array;
	string_array.data = (char*)array.data;
	string_array.length = array.length;
	return string_slice(string_array, offset, length);
}

static bool string_compare_is_equal(struct String left, struct String right) {
	i64 i;

	if (left.length != right.length) {
		return false;
	}

	for (i = 0; i < left.length; i++) {
		if (left.data[i] != right.data[i]) {
			return false;
		}
	}

	return true;
}

/*
	 This function does not handle Hex, Octal or Binary.
	 It only handles decimal

	 Returns if it succeeded and writes back into the integer pointer
*/
static bool string_parse_i64(struct String string, i64 *value) {
	i64 i;
	i64 v = 0;

	if (string.data == nil || string.length <= 0) {
		return false;
	}

	for (i = 0; i < string.length; i++) {
		char c = string.data[i];

		if (c < '0' && c > '9') {
			return false;
		}

		v *= 10;
		v += c - '0';
	}

	*value = v;
	return true;
}

#endif /* STRING_H_ */
