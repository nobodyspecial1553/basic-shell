#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include "types.h"
#include "string.h"
#include "array.h"
#include "print.h"
#include "command_parser.h"

/*
	 This project is done with pedantic C89*
	 * Backported 64 bit types because I like 64 bit types! :D
*/

int main(void) {
	/*
		 When using C, I always create a custom array structure to use everywhere
		 This way I can pass arrays instead of passing a pointer and then a length
		 This isn't as nice as other languages with better native arrays and slices, but it works.
	*/
	struct Array stdin_data = { 0 };

	/* The dynamic arrays are like the arrays, but can track how much is used (Probably better labeled an Arena, but whateva) */
	/* We're using this array to store the array of pointers to strings for execvp */
	struct Dynamic_Array args_array = { 0 };
	/* We're using this array to store all the strings that the previous array points to */
	struct Dynamic_Array args_string_array = { 0 };

	/*
		 Here we are allocating a gigabyte of memory up front to handle reading from STDIN
		 This is overkill, but I do it on purpose for the sake of example.
		 For one, this demonstrates how allocating memory isn't the same as commiting it.
		  despite asking for a Gigabyte, your system will not show a gigabyte being used (because it isn't!)
			this allows us to to not worry about reallocations and still have effectively "infinite" data
		 For two, this allows us to not have to worry about looping reads*.
		 
		 *Technically someone could pass in more than a Gig of data, but let's be real ;P
	*/
	stdin_data.length = GIGABYTE;
	/*
		 This mmap function call, like malloc, will allocate our memory
		 PROT_READ | PROT_WRITE = we can read and write to this memory
		 MAP_ANONYMOUS | MAP_PRIVATE = No backing file and changes are only visible to calling process
		 - No backing file is beyond the scope of this
		 - Changes being only visible to the calling process means if the process is forked, changes will be private to both the parent and child process instead of being shared (Search "Copy-On-Write")
		 Note: nil is a custom value I defined to represent a NULL pointer
	*/
	stdin_data.data = mmap(nil, stdin_data.length, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

	args_array.length = MEGABYTE;
	args_array.data = mmap(nil, args_array.length, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

	/* We love consuming virtual memory address space for a basic shell! YEAAAAAH!!! */
	args_string_array.length = 2 * GIGABYTE;
	args_string_array.data = mmap(nil, args_string_array.length, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

	/*
		 This is the outer-most loop which will take in data from stdin and execute the commands
	*/
	while (true) {
		static struct String input_cursor = string_static_init(">> ");
		struct Tokenizer tokenizer = { 0 };
		struct Tokenizer_Next_Result tokenizer_result = { 0 };
		u64 stdin_bytes_read = 0;

		dynamic_array_clear(&args_array);
		dynamic_array_clear(&args_string_array);

		/*
			 Prints >> and then waits for input
			 That input from stdin goes into input_data
		*/
		print(input_cursor);
		stdin_bytes_read = read(STDIN_FILENO, stdin_data.data, stdin_data.length);

		/*
			 We create a tokenizer to track the tokenization
			 Then we get the first result to check if we're doing a builtin or command
		*/
		tokenizer = tokenizer_create(string_from_array(stdin_data, 0, stdin_bytes_read));
		tokenizer_result = tokenizer_next(&tokenizer); /* <-- That's the function you're probably interested in most (command_parser.h) */
		if (tokenizer_result.error == Tokenizer_Next_Error_EOF) {
			continue;
		}
		else if (tokenizer_result.error != Tokenizer_Next_Error_None) {
			static struct String error_string = string_static_init("Error when parsing command!");
			eprintln(error_string);
			continue; /* Restart on token error */
		}

		{ /* Check if the command is a builtin and execute that */
			static struct String builtin_exit_string = string_static_init("exit");
			static struct String builtin_pwd_string = string_static_init("pwd");
			static struct String builtin_cd_string = string_static_init("cd");

			if (string_compare_is_equal(tokenizer_result.token, builtin_exit_string)) {
				struct Tokenizer_Next_Result first_arg = tokenizer_next(&tokenizer);
				i64 arg_value;

				/* We parse the first argument for its value, and if it fails, set it to zero */
				if (!string_parse_i64(first_arg.token, &arg_value)) {
					arg_value = 0;
				}

				_exit(arg_value);
			}
			if (string_compare_is_equal(tokenizer_result.token, builtin_pwd_string)) {
			}
		}

		/*
			 By moving past the builtin check, we know we now need to execute a command instead
			 This part isn't relevant to what you asked (notrobinnot) so you probably don't need to look past it.
		*/
		{
			pid_t pid = 0;

			/* Turn command into C style string */
			{
				/* Get end of the array to write our data to
					 I'm doing this byte by byte by hand instead
					 of using dynamic_array_push to not
					 use a silly function call every byte
				*/
				char *str_ptr = (char*)&args_string_array.data[args_string_array.length];
				i64 i;

				/* Copy bytes, byte by byte, from the token into the string buffer array */
				for (i = 0; i < tokenizer_result.token.length; i++) {
					str_ptr[i] = tokenizer_result.token.data[i];
				}
				str_ptr[i++] = '\0'; /* Add null terminator */
				args_string_array.length += i;
				
				/* Add pointer to copied string into args array for execvp */
				dynamic_array_push(&args_array, &str_ptr, 1);
			}

			/* Turn args into C style strings */
			while (true) {
				struct Tokenizer_Next_Result result = tokenizer_next(&tokenizer);
				char *str_ptr = (char*)&args_string_array.data[args_string_array.length];
				i64 i;

				/*
					 This breaks out on any error
					 Hypothetically you'd want to do different stuff on different errors
					 and only clean break on EOF
					 Skipping it for brevity
				*/
				if (result.error != Tokenizer_Next_Error_None) {
					break;
				}

				for (i = 0; i < result.token.length; i++) {
					str_ptr[i] = result.token.data[i];
				}
				str_ptr[i++] = '\0';
				args_string_array.length += i;
				
				dynamic_array_push(&args_array, &str_ptr, 1);
			}
			{ /* Pop nil onto end */
				char *terminator = nil;
				dynamic_array_push(&args_array, &terminator, 1);
			}

			/* Fork and execute */
			pid = fork();
			switch (pid) {
			case -1:
				{
					static struct String fork_error_message = string_static_init("Failed to fork command!");
					eprintln(fork_error_message);
					continue;
				}
			case 0: /* Child process enters here */
				{
					static struct String failed_exec_string = string_static_init("Failed to start command!");
					execvp(*((char**)args_array.data), (char**)args_array.data);
					eprintln(failed_exec_string);
					_exit(1); /* Failed to execute */
				}
			default: /* Parent process */
				while (true) {
					int status = 0;
					pid_t wpid = waitpid(pid, &status, 0);

					if (wpid == -1) {
						static struct String wait_error_string = string_static_init("Failed to wait on child process!");
						eprintln(wait_error_string);
						break;
					}

					if (WIFEXITED(status)) {
						break;
					}
					else if (WIFSIGNALED(status)) {
						break;
					}
				}
			}
		}
	}

	return 0;
}
