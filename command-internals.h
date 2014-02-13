
// UCLA CS 111 Lab 1 command internals

enum command_type
{
    // Might not be necessary but do it just in case
    AND_COMMAND,         
    SEQUENCE_COMMAND,    
    OR_COMMAND,          
    PIPE_COMMAND,        
    SIMPLE_COMMAND,      
    SUBSHELL_COMMAND,
    NOT_A_COMMAND_YET,    
};

// Data associated with a command.
struct command
{
    enum command_type type;
    // int priority;

    // Exit status, or -1 if not known (e.g., because it has not exited yet).
    int status;

    // I/O redirections, or 0 if none.
    char *input;
    char *output;

    union
    {
        // for AND_COMMAND, SEQUENCE_COMMAND, OR_COMMAND, PIPE_COMMAND:
        struct command *command[2];

        // for SIMPLE_COMMAND:
        char **word; // a pointer to strings

        // for SUBSHELL_COMMAND:
        struct command *subshell_command;
    } u;
};

struct command_stream
{
    // command_t* carray; // pointer to an array of command_t objects, 
                       // which are just pointers to 'struct command's

    command_t carray;
    int stream_index;
    // command_t* waitingzone; // for incomplete commands
    // Better implementation of waiting zone is just one command
    struct command* waitingzone;
    int wi; // waitingzone indicator, 1 if waiting, 0 if not waiting
    
    int timesread;
    //int numcmds; // number of commands in a stream
    // int* hp; // highest priority in each line?? hp[0] = 5; hp[1] = 3;

    // For cases like
    // a<b>c|d<e>f|g<h>i
    // we're going to need to know how many <'s >'s and |'s there are AHEAD of time

    // To be used for checking syntax
    int valid_syntax; // -1 if bad, 0 if good
    int num_parentheses; // balanced open and closed parentheses, ++ if ( and -- if )
                              // desired result should therefore be 0
};
/* **********************
 * COMMAND STACK STRUCT *
 ************************/ 

// Stack struct with an array of command structs
typedef struct 
{
    struct command* con;    // SEE command-internals.h
    int top;        // -1 = EMPTY
    int capacity;
} cmd_stack_struct;


////////////////////////////////////////////////////////////////////////////////////////////////////////
// TOKENS ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

enum token_type
{
    WORD,
    LOGICAND,
    LOGICOR, // && and ||
    SUBSHELL,
    COMMENT,
    I,
    O,
    PIPE,
    NEWLINE,
    SEMICOLON,
    INITIAL,
    //EOF,
};

typedef struct
{
    enum token_type type;
    char* data; // to hold what it actually is
    // union
    // {
    //     char* word; // For words and operators
    //     char symbol; // For everything else, if needed
    // } data;
} token;

typedef struct
{
    token* tokarray;
    int size; // size of token array
} token_stream;

/**********************
 * TOKEN STACK STRUCT *
 **********************/

// Stack struct with an array of token structs
typedef struct 
{
    token *con;
    int top;    // -1 = EMPTY
    int capacity;
    //int *holder;    // an array of ints that holds the place of the operator 
} tok_stack_struct;

