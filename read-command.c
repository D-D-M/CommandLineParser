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
	command_t* carray; // array of commands 

	// To be used for checking syntax
	int valid_syntax;
	int num_parentheses; // balanced open and closed parentheses, ++ if ( and -- if )
							  // desired result should therefore be 0
};


// This function picks up from where the stream left off, gets the next word in line
// (only called when we have reason to believe that there is a word next in line), and
// then adds it to the wordbuffer.

char* read_word(int (*get_next_byte) (void *), void *get_next_byte_argument,
						char byte, int wordcap, int wordbuffercap, char** wordbuffer, int* j)
{
	int i;
	char* word = checked_malloc(sizeof(char)*wordcap); // holds an individual word
	i = 0;
	while (isalnum(byte) || byte == '!' || byte == '%' || byte == '+' || byte == ',' ||
		  byte == '-' || byte == '.' || byte == '/' || byte == ':' || byte == '@' ||
		  byte == '^' || byte == '_')
	{
		if (i >= wordcap-1) // Reallocate if not enough room
		{
			wordcap = wordcap * 2;
			word = checked_realloc(word, wordcap);
		}
		word[i] = byte;
		byte = get_next_byte(get_next_byte_argument);
		i++;
	}
	// add the word to the wordbuffer
	word[i] = '\0'; // important to add the null terminator first
	if (*j >= wordbuffercap-1) // Reallocate if not enough room
	{
		wordbuffercap = wordbuffercap * 2;
		wordbuffer = checked_realloc(wordbuffer, wordbuffercap);
	}
	wordbuffer[*j] = word;
	*j++;
	byte = get_next_byte(get_next_byte_argument);
	return word;
}


command_stream_t make_command_stream (int (*get_next_byte) (void *),
			  void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
	  add auxiliary functions and otherwise modify the source code.
	  You can also use external functions defined in the GNU C Library.  */
   char byte;
   char next;
   int i;
   int* j = 0;
   *j=0;
   int wordlength;
   int count = 0;

   int wordcap = 8; // arbitrary guess
   int wordbuffercap = 4;
  
   // Using the checked_malloc function provided. If it doesn't work, check your own malloc.
   command_stream_t stream = (command_stream_t)checked_malloc(sizeof(struct command_stream));
   stream->valid_syntax = 1; // TRUE
   
   char** wordbuffer = checked_malloc(sizeof(char*)*wordbuffercap); // holds the words that come before a command

   byte = get_next_byte(get_next_byte_argument); 
  
	while( byte != EOF )
  	{
		// ! % + , - . / : @ ^ _ are allowed as well
		if (isalnum(byte) || byte == '!' || byte == '%' || byte == '+' || byte == ',' ||
				  byte == '-' || byte == '.' || byte == '/' || byte == ':' || byte == '@' ||
				  byte == '^' || byte == '_')
		{
			char* word = read_word(get_next_byte, get_next_byte_argument,
						byte, wordcap, wordbuffercap, wordbuffer, j);
			// put the word on the word stack
		}
		else
		{
			// EVERY COMMAND will need to reset the j variable after 
			// assimilating its contents into the command type
			switch(byte)
			{
				// WHITESPACE
				case ' ':
					byte = get_next_byte(get_next_byte_argument);
					*j = 0;
					count++;
					break;
				case ';':
					byte = get_next_byte(get_next_byte_argument);
					count++;
					*j = 0;
					break;
				case '<':
					stream->carray[count]->type = SIMPLE_COMMAND;
					stream->carray[count]->input = read_word(get_next_byte, get_next_byte_argument,
						byte, wordcap, wordbuffercap, wordbuffer, j); // read_word() to the right of <
					stream->carray[count]->output = NULL;
					stream->carray[count]->u.word = wordbuffer;
					count++;
					*j = 0;
					break;

				case '>':
					stream->carray[count]->type = SIMPLE_COMMAND;
					stream->carray[count]->output = read_word(get_next_byte, get_next_byte_argument,
						byte, wordcap, wordbuffercap, wordbuffer, j); // read_word() to the right of >
					stream->carray[count]->input = NULL;
					stream->carray[count]->u.word = wordbuffer;
					*j = 0;
					count++;
					break;


				// AMPERSAND SYMBOL
				case '&':
					next = get_next_byte(get_next_byte_argument);
					if (next == '&')
					{
						stream->carray[count]->type = AND_COMMAND;
						stream->carray[count]->input = NULL;
						stream->carray[count]->output = NULL;
						// if (count == 0) // can we just assume invalid syntax?
						// stream->carray[count]->u.command = ;
						
					}
					else
					{
						stream->valid_syntax = 0; // set invalid syntax
					}
					*j = 0;
					count++;
					break;
				// PIPE SYMBOL
				case '|':
					next = get_next_byte(get_next_byte_argument);
					if (next == '|')
					{
						stream->carray[count]->type = OR_COMMAND;
						// stream->carray[count]->input = ;
						// stream->carray[count]->output = ;
						// stream->carray[count]->u.command = ;
				
					}
					else if (next == ' ')
					{
						stream->carray[count]->type = PIPE_COMMAND;
						// stream->carray[count]->input = ;
						// stream->carray[count]->output = ;
						// stream->carray[count]->u.command = ;
						
					}
					else
					{
						stream->valid_syntax = 0; // set invalid syntax
					}
					*j = 0;
					count++;
					break;
				case '\n':
					// byte = get_next_byte(get_next_byte_argument);
					*j = 0;
					count++;
					break;
				case '(':
					byte = get_next_byte(get_next_byte_argument);
					*j = 0;
					count++;
					break;
				case ')':
					byte = get_next_byte(get_next_byte_argument);
					*j = 0;
					count++;
					break;
			}
			
		}

	}
	// error (1, 0, "command reading not yet implemented");
	return 0;
}



command_t
read_command_stream (command_stream_t s)
{
	/* FIXME: Replace this with your implementation too.  */
	error (1, 0, "command reading not yet implemented");
	return 0;
}
