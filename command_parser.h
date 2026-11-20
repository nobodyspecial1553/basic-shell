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

enum Tokenizer_Next_Error {
	Tokenizer_Next_Error_None = 0,
	Tokenizer_Next_Error_EOF,
	Tokenizer_Next_Error_Invalid_Token,
	Tokenizer_Next_Error_Generic_Error /* For laziness */
};

struct Tokenizer_Next_Result {
	struct String token;
	enum Tokenizer_Next_Error error;
};

/*
	 Returns the next token
*/
static struct Tokenizer_Next_Result tokenizer_next(struct Tokenizer *tokenizer) {
	struct Tokenizer_Next_Result result = { 0 };

	/*
		 Skipping past whitespace
	*/
	while (true) {
		switch (tokenizer->command.data[tokenizer->index]) {
		case ' ': /* Cases in C fall through to the next case if no break stops them */
		case '\t':
		case '\r':
		case '\n':
		case '\a':
			if (++tokenizer->index >= tokenizer->command.length) { /* Hit EOF before finding token */
				result.error = Tokenizer_Next_Error_EOF;
				return result;
			}
			continue; /* Starts the loop again, pushing the index one forward */
		}
		break; /* If the switch ever fails, then we know there is no more whitespace */
	}

	/*
		 Here we decide how to parse the text
		 If we encounter a ' or ", then we parse all the text between quotations as one token
		 Otherwise we parse until whitespace
	*/
	switch (tokenizer->command.data[tokenizer->index]) {
	case '\'': /* This falls through to the next case */
	case '"':
		{
			char opening_quotation = tokenizer->command.data[tokenizer->index];
			i64 token_start_index = ++tokenizer->index; /* Move one past the opening quotation */

			/* Loop through until we find closing quotation */
			while (true) {
				if (tokenizer->command.data[tokenizer->index] == opening_quotation) {
					break;
				}

				if (++tokenizer->index >= tokenizer->command.length) {
					result.error = Tokenizer_Next_Error_EOF;
					return result;
				}
			}

			/*
				 - set the token string to the offset and length found
				 - move the index past the closing quotation
			*/
			result.token.data = &tokenizer->command.data[token_start_index];
			result.token.length = tokenizer->index - token_start_index;
			tokenizer->index++;
			return result;
		}
	default: /* This is the generic case */
		{
			i64 token_start_index = tokenizer->index;

			/*
				 Loop through until we find whitespace, which will tell us we're at the end of the token
			*/
			for (;; tokenizer->index++) {
				if (tokenizer->index >= tokenizer->command.length) {
					break; /* EOF is also a valid end to the token */
				}

				switch (tokenizer->command.data[tokenizer->index]) {
				case ' ':
				case '\t':
				case '\r':
				case '\n':
				case '\a':
					break;
				default:
					continue; /* Restart loop as we haven't found whitespace */
				}
				break; /* If the switch ever fails, we know we found whitespace */
			}

			result.token.data = &tokenizer->command.data[token_start_index];
			result.token.length = tokenizer->index - token_start_index;
			return result;
		}
	}

	/*
		 Should be unreachable as all other paths end in a return
	*/
	result.error = Tokenizer_Next_Error_Generic_Error;
	return result;
}

#endif /* COMMAND_PARSER_H_ */
