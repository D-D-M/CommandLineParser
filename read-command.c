// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <error.h>
// Add your own include directives
#include <stdlib.h> // malloc the command stream
#include <stdio.h> // end of file
#include "alloc.h" // checked malloc
#include <ctype.h> // checking for alpha numeric characters
/*
	In Lab 1a, you'll warm up by implementing just the shell's command reader. 
	This shell will support a -p option, so that the command "timetrash -p script.sh" 
	will read the shell commands in the file script.sh and output them in a standard 
	format that is already supplied by a code skeleton available on CourseWeb; 
	sample output is in the skeleton's test script test-p-ok.sh.
*/


/* FIXME: You may need to add #include directives, macro definitions,
	static function definitions, etc.  */

// #include "stack.h" // doesn't work with 'make' command

/* FIXME: Define the type 'struct command_stream' here.  This should
	complete the incomplete type declaration in command.h.  */

// Words, consisting of a maximal sequence of one or more adjacent characters that are 
// ASCII letters (either upper or lower case), digits, or any of: ! % + , - . / : @ ^ _

// Not everything that we parse will be a command, per se. A more general and inclusive
// term would be tokens. Make it just like the command_type in command-internals.h, but
// hopefully better.

struct command_stream
{
	command_t carray; // array of commands 

	// To be used for checking syntax
	int valid_syntax;
	int num_parentheses; // balanced open and closed parentheses, ++ if ( and -- if )
							  // desired result should therefore be 0
};

// This function determines if a char c can be part of a word, 
// according to our shell grammar/syntax.
int is_word(const char* c)
{
	if (isalnum(*c) || *c == '!' || *c == '%' || *c == '+' || *c == ',' || *c == '-' || 
		  *c == '.' || *c == '/' || *c == ':' || *c == '@' || *c == '^' || *c == '_')
		return 1; // TRUE, this char can be part of a word
	else
		return 0; // FALSE, this car can NOT be part of a word
}

// This time, our implementation will be to read one line at a time
command_stream_t make_command_stream (int (*get_next_byte) (void *),
			  void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
	  add auxiliary functions and otherwise modify the source code.
	  You can also use external functions defined in the GNU C Library.  */
    
    int i; // for 'for' loops
    
    // Allocate space for the command stream
    // NOTE: A command_stream_t is a POINTER to a struct command_stream
    command_stream_t stream = (command_stream_t)checked_malloc(sizeof(struct command_stream));
    
    // A command_stream_t has an array pointer, so we're going to need to malloc that as well
    size_t commandcap = 100; // Assume a file will have no more than 100 commands
    int cmd_count = 0; // Counter for the number of commands in one file
    stream->carray = (command_t)checked_malloc(sizeof(struct command)*commandcap);
	stream->valid_syntax = 1;
	// stream->num_parentheses = 0;

	int line_cap = 200; // Assume a line will have no more than 200 characters
	int line_count = 0; // Counter for the number of characters in one line
	char* line = (char *)checked_malloc(sizeof(char)*line_count);

    char byte = get_next_byte(get_next_byte_argument);
    // As long as we haven't reached the end of the file, keep getting the next character
    while (byte != EOF)
    {
    	while (byte != '\n')
    	{
    		// Doesn't matter what the byte is, just put it in the line
    		line[line_count] = byte;
    		line_count++;

    		// If a line is longer than we expect, update its capacity and reallocate.
    		if (line_count >= line_cap)
    		{
    			line_cap *= 2;
    			line = checked_realloc(line, line_cap);
    		}
    		byte = get_next_byte(get_next_byte_argument);
    	}
    	// Exiting this loop means that byte = newline, and we can begin with:
    	// 1. If line_count != 0,
    	// 2. Try parsing the command
    	// 3. Then put that command into 'stream[cmd_count]'
    	// 4. cmd_count++;

    	// Reset everything
    	line_count = 0;
    	// If we haven't done so already, update byte
    	byte = get_next_byte(get_next_byte_argument);
    }

	return 0;
}



command_t
read_command_stream (command_stream_t s)
{
	/* FIXME: Replace this with your implementation too.  */
	error (1, 0, "command reading not yet implemented");
	return 0;
}
