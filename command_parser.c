#ifndef COMMAND_PARSER_H_
#define COMMAND_PARSER_H_

#include "types.h"
#include "string.h"
#include "array.h"

struct Tokenizer {
	struct String command;
	i64 index;
};

static struct Tokenizer tokenizer_create(struct String command_string) {
	struct Tokenizer tokenizer = { 0 };

	tokenizer.command = command_string;
	tokenizer.index = 0;

	return tokenizer;
}

/*
	 Returns the next token
*/
static struct String tokenizer_next(struct Tokenizer *tokenizer) {
	struct String token;
	char *c;

	assert(tokenizer != nil);

	return token;
}

#endif /* COMMAND_PARSER_H_ */
