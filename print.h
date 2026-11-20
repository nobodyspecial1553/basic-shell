#ifndef PRINT_H_
#define PRINT_H_

#include <unistd.h>

#include "types.h"
#include "string.h"

static void fprint(i32 fd, struct String string) {
	write(fd, string.data, string.length);
}

static void fprintln(i32 fd, struct String string) {
	write(fd, string.data, string.length);
	write(fd, "\n", 1);
}

static void print(struct String string) {
	fprint(STDOUT_FILENO, string);
}

static void println(struct String string) {
	fprintln(STDOUT_FILENO, string);
}

static void eprint(struct String string) {
	fprint(STDERR_FILENO, string);
}

static void eprintln(struct String string) {
	fprintln(STDERR_FILENO, string);
}

#endif /* PRINT_H_ */
