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

/* FIXME: Define the type 'struct command_stream' here.  This should
	complete the incomplete type declaration in command.h.  */

// Words, consisting of a maximal sequence of one or more adjacent characters that are 
// ASCII letters (either upper or lower case), digits, or any of: ! % + , - . / : @ ^ _


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
		return 0; // FALSE, this char can NOT be part of a word
}

// This function determines if a char c can is part of a command
int is_cmd(const char* c)
{
	if (*c == '|' || *c == '&' || *c == '(' || *c == ')' || *c == ';' || 
		*c == '<' || *c == '>')
		return 1; // TRUE, this char is a command
	else
		return 0; // FALSE, this char is NOT a command
}

// This time, our implementation will be to read one line at a time
command_stream_t make_command_stream (int (*get_next_byte) (void *),
			  void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
	  add auxiliary functions and otherwise modify the source code.
	  You can also use external functions defined in the GNU C Library.  */
    
    int i; // for various loops
    
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
    	// Don't forget that if byte == '#', we should essentially ignore everything
    	// in between # and the next newline, because it's just a comment and we don't
    	// care about those!
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

    	if (line_count != 0) // Parse the command if there's a command to parse.
    	{
    		// First good idea would be to iterate through the array to see what
    		// our highest priority command is, and then base all of the other
    		// commands off of that one.

    		// Make a variable to keep track of the highest priority command in 'line'
    		command_t highest_priority = SIMPLE_COMMAND; // start as low as possible, work your way up from there
    		// This for loop is just for FINDING the highest priority command in the line
    		for (i = 0; i < line_count; i++)
    		{
    			if (is_cmd(line[i])) // if it's a command...
    			{
    				switch (line[i])
    				{
    					case ';':
    						if (highest_priority < SEQUENCE_COMMAND)
    							highest_priority = SEQUENCE_COMMAND;
    						break;
    					case '&':
    						// Need to check that the next char is also &
    						char next = get_next_byte(get_next_byte_argument);
    						if (next == '&')
    						{
    							if (highest_priority < AND_COMMAND)
    								highest_priority = AND_COMMAND;
    						}
    						else
    						{
    							stream->valid_syntax = 0; // invalid syntax
    							// Perhaps a goto statement?
    							// Who knows, maybe this is still valid syntax
    						}
    						break;
    					case '|':
    						char next = get_next_byte(get_next_byte_argument);
    						if (next == ' ' || is_word(next)) // PIPE
    						{
    							if (highest_priority < PIPE_COMMAND)
    								highest_priority = PIPE_COMMAND;
    							// break;	
    						}
    						else if (next = '|') // OR
    						{
                                if (highest_priority < OR_COMMAND)
                                    highest_priority = OR_COMMAND;
                                // break;
    						}
    						else
    						{
    							stream->valid_syntax = 0;
                                // Pretty sure this is also invalid syntax.
    						}
    						break;
                        case '<':
                        case '>':
                            if (highest_priority < IO_COMMAND)
                                highest_priority = IO_COMMAND;
                            break;
                            
                        // For a subshell, it might be a good idea to just search the
                        // line for a '(' explicitly, and then decide what we need to do
                        // about it from there. Might have a helper function to evaluate
                        // subshell bullshit.
                        case '(':
                            if (highest_priority < SUBSHELL_COMMAND)
                                highest_priority = SUBSHELL_COMMAND;
                            break;
    				}
    			}
    		}
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

	return stream;
}



command_t
read_command_stream (command_stream_t s)
{
	/* FIXME: Replace this with your implementation too.  */
	
	// Probably going to need to use recursion for subshells, but we'll see...
	error (1, 0, "command reading not yet implemented");
	return 0;
}
