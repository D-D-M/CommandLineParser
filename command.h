// UCLA CS 111 Lab 1 command interface

typedef struct command* command_t;
typedef struct command_stream* command_stream_t;


/* Create a command stream from LABEL, GETBYTE, and ARG.  A reader of
   the command stream will invoke GETBYTE (ARG) to get the next byte.
   GETBYTE will return the next input byte, or a negative number
   (setting errno) on failure.  */
command_stream_t make_command_stream (int (*getbyte) (void *), void *arg);

/* Read a command from STREAM; return it, or NULL on EOF.  If there is
   an error, report the error and exit instead of returning.  */
command_t read_command_stream (command_stream_t stream);

/* Print a command to stdout, for debugging.  */
void print_command (command_t);

/* Execute a command.  Use "time travel" if the integer flag is
   nonzero.  */
void execute_command (command_t, int);

/* Return the exit status of a command, which must have previously been executed.
   Wait for the command, if it is not already finished.  */
int command_status (command_t);


// POTENTIAL PARSE CODE
/*
    int wordscap = 10;
    int charscap = 20;
    int w = 0;
    int wa = 0;
    char** words = (char**)checked_malloc(sizeof(char*)*wordscap);
    for (; c < size; c++)
    {
        char* word = (char*)checked_malloc(sizeof(char)*charscap);
        while(is_word(&line[c]))
        {
            word[w] = line[c];
            c++; w++;
        }
        words[wa] = word;
        w = 0; // reset the w

        // See what the next char is (why wasn't it a word)
        // if (line[c] == '<' || line[c] == '>')
        

    }
*/
