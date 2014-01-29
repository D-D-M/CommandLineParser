// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <error.h>
// Add your own include directives
#include <stdlib.h> // malloc the command stream
#include <stdio.h> // end of file
#include "alloc.h" // checked malloc
#include <ctype.h> // dchecking for alpha numeric characters
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
    command_t* carray; // pointer to an array of command_t objects, 
                       // which are just pointers to 'struct command's
    command_t* waitingzone; // for incomplete commands
    
    int timesread;
    int numcmds; // number of commands in a stream
    // int* hp; // highest priority in each line?? hp[0] = 5; hp[1] = 3;

    // For cases like
    // a<b>c|d<e>f|g<h>i
    // we're going to need to know how many <'s >'s and |'s there are AHEAD of time

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
    //EOF,
};
typedef struct
{
    enum token_type type;
    union
    {
        char* word; // For words and operators
        char symbol; // For everything else, if needed
    } data;
} token;
typedef struct
{
    token* tokarray;
    int size; // size of token array
} token_stream;

// If it succeeds, this function accepts a char array and returns a token_stream
// If it fails (because of bad syntax), this function will return NULL.
token_stream* tokenize(const char* line, const int linesize)
{
    int sizeguess = 50;
    token_stream* ts = (token_stream*)checked_malloc(sizeof(token_stream));
    // These will be the fields of the token stream at the very end (easier to read)
    token* a = (token*)checked_malloc(sizeof(token)*sizeguess);
    int tasize = 0;

    int nt = 0; // number of tokens
    int c = 0; // which char in line[c]
    int w;
    // cat < a || ls foo > b
    for (; c < linesize; )
    {
        if (is_word(&line[c]))
        {
            int w = 0;
            int charscap = 20;
            char* word = (char*)checked_malloc(sizeof(char)*charscap);
            while (is_word(&line[c]))
            {
                word[w] = line[c];
                w++; c++;
                // printf("%c\n", line);
            }
            // word[w] = '\0'; // null terminator
            // printf("%s\n",word);
            // Put that word in the tokenarray, update token
            // printf("tasize %d\n", tasize);
            a[tasize].type = WORD;
            a[tasize].data.word = word;
            tasize++;
        }
        else if (line[c] == '#')
            c = linesize; // Comment means we can stop
        else if (line[c] == '<') // IO
        {
            a[tasize].type = I;
            a[tasize].data.symbol = line[c];
            c++; tasize++;
        }
        else if (line[c] == '>')
        {
            a[tasize].type = O;
            a[tasize].data.symbol = line[c];
            c++; tasize++;
        }
        else if (line[c] == '|')
        {
            if (line[c+1] == '|') // if the next one is also a pipe, we've got OR
            {
                a[tasize].type = LOGICOR;
                a[tasize].data.word = "||\0";
                c += 2; 
                tasize++;
            }
            else // otherwise, we have a pipe
            {
                a[tasize].type = PIPE;
                a[tasize].data.symbol = line[c];
                c++; tasize++;
            }
        }
        else if (line[c] == '&')
        {
            if (line[c+1] == '&')
            {
                a[tasize].type = LOGICAND;
                a[tasize].data.word = "&&\0";
                c += 2;
                tasize++;
            }
            else
                return NULL; // Return NULL, our shell doesn't support this grammar
        }
        else if (line[c] == ';')
        {
            a[tasize].type = SEMICOLON;
            a[tasize].data.symbol = line[c];
            c++; tasize++;
        }
        else if (line[c] == '(' || line[c] == ')')
        {   
            a[tasize].type = SUBSHELL;
            a[tasize].data.symbol = line[c];
            c++; tasize++;
        }
        else if (line[c] == '\n')
        {
            a[tasize].type = NEWLINE;
            a[tasize].data.symbol = line[c];
            c++; tasize++;
        }
        else
            c++; // For now, we just won't put it into the tokenarray  
    }

    // Set the tokenstream fields
    ts->tokarray = a;
    ts->size = tasize;
    return ts;
}

/*************************
// I STARTED HERE
**************************/

int is_op(const token *t)
{
    if (t->type == PIPE || t->type == LOGICOR || t->type == LOGICAND || t->type == SEMICOLON)
        return 1;
    else 
        return 0;
}

int is_simple(const token *t)
{
    if (t->type == WORD || t->type == I || t->type == O)
        return 1;
    else 
        return 0;
}

int precedence(const token t)
{
    switch(t.type)
    {
        case PIPE:
          return 3;
        case LOGICOR:
        case LOGICAND:
          return 2;
        case SEMICOLON:
          return 1; 
        case WORD:
        case SUBSHELL:
        case COMMENT:
        case I:
        case O:
        case NEWLINE:
         return 0;
    }
    return -1;       
}

/* **********************
 * COMMAND STACK STRUCT *
 ************************/ 

// Stack struct with an array of command structs
typedef struct 
{
    struct command* con;    // SEE command-internals.h
    int top;        // -1 = EMPTY
    //int size;
} cmd_stack_struct;

// Stack function - PUSH
void push_cmd(cmd_stack_struct *stack, struct command content)
{
    stack->top++;
    stack->con[stack->top] = content;
}

// Stack fucntion - POP
// returns a command 
struct command pop_cmd(cmd_stack_struct *stack)
{
    int poptop = stack->top;
    stack->top--;
    return stack->con[poptop];
}


/**********************
 * TOKEN STACK STRUCT *
 **********************/

// Stack struct with an array of token structs
typedef struct 
{
    token *con;
    int top;    // -1 = EMPTY
    //int size;
    //int *holder;    // an array of ints that holds the place of the operator 
} tok_stack_struct;

// Stack function - PUSH
void push_tok(tok_stack_struct *stack, token content)
{
    stack->top++;
    stack->con[stack->top] = content;
}

// Stack fucntion - POP
// returns a token
token pop_tok(tok_stack_struct *stack)
{
    int poptop = stack->top;
    stack->top--;
    return stack->con[poptop];
}


/*********
 * PARSE *
 *********/
// Because any command can continue onto the next line, and we're only reading one line at a time,
// we're going to need access to the command stack and the operator stack 
// OUTSIDE of this function, OUTSIDE of even the while !EOF loop
// 
// Shit, maybe even a command_stream_t needs to declare both stacks in itself,
// yeah, that's good, I love it

struct command* parse(token_stream *ts, cmd_stack_struct* cmdstack, tok_stack_struct* opstack)
{   
    // Simplify
    int maxsize = ts->size;
    token* tarray = ts->tokarray;
    // This function will return 'root'
    struct command* root = (command_t)checked_malloc(sizeof(struct command)*maxsize);

    int ti;                     // token iterator 

    // Iterate through the token array
    for(ti = 0; ti < maxsize; ti++)
    {
        //
        // PSEUDOCODE begins here
        //
        struct command* cmd = (struct command*)checked_malloc(sizeof(struct command));
        // cmd->status = 0;
        //
        if (is_simple(&tarray[ti]))
        {
            int z = 0;
            cmd->u.word = (char**)checked_malloc(sizeof(char*)*maxsize);
            while(tarray[ti].type == WORD)
            {
                // Word isn't being set properly
                cmd->u.word[z] = tarray[ti].data.word;
                ti++; z++;
            }
            // INPUT < SYMBOL
            if (tarray[ti].type == I)
            {
                if (tarray[ti+1].type == WORD)
                {
                    // Furthermore, if the NEXT next token is >
                    if (tarray[ti+2].type == O)
                    {
                        cmd->input = tarray[ti+1].data.word;
                        if (tarray[ti+3].type == WORD)
                        {
                            cmd->output = tarray[ti+3].data.word;
                            cmd->status = 1;
                            ti+=2; // Just trust me on this one
                        }
                        else
                        {
                            cmd->status = 0;
                        }
                        ti++;
                    }
                    else // THE ELSE IS WHAT WORKED BEFORE
                    {
                        cmd->input = tarray[ti+1].data.word;
                        cmd->output = NULL;
                        ti++;
                        // ti+=2;
                        cmd->status = 1; // complete command
                    }
                }
                else
                {
                    if (tarray[ti+1].type == NEWLINE)
                        cmd->status = 0; // incomplete command
                    else
                        cmd->status = -1; // error (we think) might just return NULL;
                }
            }
            // OUTPUT > SYMBOL
            else if (tarray[ti].type == O)
            {
                if (tarray[ti+1].type == WORD)
                {
                    // // Only set input to NULL if cmd->status is 0, if it's incomplete
                    // if (cmd->status == 0)
                    // {
                    //     cmd->input = NULL;
                    // }
                    // else // a b<c > d
                    // {
                    //     // Input has already been set by the input operator above, so we're good
                    //     ;
                    // }
                    cmd->input = NULL;
                    cmd->output = tarray[ti+1].data.word;
                    ti++;
                    cmd->status = 1; // complete command
                }
                else
                {
                    if (tarray[ti+1].type == NEWLINE)
                        cmd->status = 0; // incomplete command
                    else
                        cmd->status = -1; // error (we think)
                }
            }
            else if (tarray[ti].type == NEWLINE || tarray[ti].type == SEMICOLON)
            {
                cmd->status = 1;
                ti--;
            }
            else
                ti--;
            cmd->type = SIMPLE_COMMAND;
            // Now we either have a simple command with a status
            push_cmd(cmdstack, *cmd);
            printf("Stack push_cmd!\n");
            // Free that command
            // free(cmd);
        }
        else // if (is_op(&ts->tokarray[ti]))     // is an operator: &&, ||, |, ;
        {
            // If the opstack is empty, just push this operator onto it, badaboom
            if (opstack->top == -1)
            {
                printf("Pushing op at position %d !\n", ti);
                push_tok(opstack, tarray[ti]);
            }
            // If the precedence of the operator on the opstack is greater than the precedence
            // of the token you've just run into, you need to start linking commands up.
            else if (precedence(opstack->con[opstack->top]) >= precedence(tarray[ti]) || ti >= maxsize)
            {
                // What we had before...
                // struct command lr[2];  // left and right commands to be joined // FISHYYYY
                // struct command* lr[2];  // left and right commands to be joined // FISHYYYY
                
                // But now I want to make sure I malloc properly, I want to have CONTROL
                // struct command** lr = (struct command**)checked_malloc(sizeof(struct command*)*2); // 2, for L and R
                
                // Final idea
                struct command* L = (struct command*)checked_malloc(sizeof(struct command));
                struct command* R = (struct command*)checked_malloc(sizeof(struct command));

                switch(opstack->con[opstack->top].type)
                {
                    case PIPE:
                        cmd->type = PIPE_COMMAND;
                        break;
                    case LOGICOR:
                        cmd->type = OR_COMMAND;
                        break;
                    case LOGICAND:
                        cmd->type = AND_COMMAND;
                        break;
                    case SEMICOLON:
                        cmd->type = SEQUENCE_COMMAND;
                        break;
                    case NEWLINE:
                        // cmd->type = SEQUENCE_COMMAND;
                        break;
                    case WORD:
                    case SUBSHELL:
                    case COMMENT:
                    case I:
                    case O:
                        printf("Parse returning NULL now\n");
                        return NULL; 
                }
                cmd->input = NULL;
                cmd->output = NULL;
                cmd->status = 0; // Command status should be "waiting" until we know that we have 
                                // two commands on the stack, ready to be joined
                // If the stack is NOT empty
                if (cmdstack->top != -1)
                {
                    // Set the value of the command
                    *R = pop_cmd(cmdstack);
                    if (cmdstack->top != -1)
                    {
                        *L = pop_cmd(cmdstack);
                        cmd->u.command[0] = L;
                        cmd->u.command[1] = R;
                        cmd->status = 1; // Not waiting anymore
                    }
                    else
                    {
                        cmd->u.command[0] = R; // still waiting
                    }
                }
                else // The stack is empty
                {
                    printf("operator with no subcommands\n");
                    cmd->status = -1; // error
                }

                push_cmd(cmdstack, *cmd);
                // free(cmd); // Free the command now that you've pushed its value onto the cmdstack
                pop_tok(opstack);
                printf("tarray[%d] type is %d\n", ti, tarray[ti].type);
                ti = ti-1;
                /*
                if (tarray[ti].type == NEWLINE || tarray[ti].type == SEMICOLON)
                    ; // Do nothing
                else
                    ti--;   // Decrement - This makes sense, don't make Eskild explain it to you again.                                  
                */
            }
            else
            {
                printf("Pushing op at position %d !\n", ti); 
                push_tok(opstack, tarray[ti]);
            }
        }
    }

    *root = pop_cmd(cmdstack);
    // Test what the value of root is before we return it
    printf("root type is %d\n",root->type);
    printf("root status is %d\n", root->status);
    // if (root->)
    // printf("Is root null? Check: %p\n", root);
    return root;
}

// This time, our implementation will be to read one line at a time
command_stream_t make_command_stream (int (*get_next_byte) (void *),
              void *get_next_byte_argument)
{
    int i;
    int stream_index = 0;
    int waiting_index = 0;
    // Allocate space for the command stream
    // NOTE: A command_stream_t is a POINTER to a struct command_stream
    command_stream_t stream = (command_stream_t)checked_malloc(sizeof(struct command_stream));
    // Initialize command stream
    int commandcap = 100; // Assume a file will have no more than 100 commands
    int cmd_count = 0; // Counter for the number of commands in one file
    int cmd_it = 0; // iterator for finding the next command in a line
    stream->carray = (command_t*)checked_malloc(sizeof(command_t)*commandcap);
    stream->valid_syntax = 1;
    stream->waitingzone = (command_t*)checked_malloc(sizeof(command_t)*commandcap);

    // We need a commandstack, and we need an operatorstack
    cmd_stack_struct* cmdstack = (cmd_stack_struct*)checked_malloc(sizeof(cmd_stack_struct));
    tok_stack_struct* opstack = (tok_stack_struct*)checked_malloc(sizeof(tok_stack_struct));
    int stacksizes = 100;

    cmdstack->con = (struct command*)checked_malloc(sizeof(struct command)*(stacksizes));
    cmdstack->top = -1;  // EMPTY
    opstack->con = (token*)checked_malloc(sizeof(token)*(stacksizes));
    opstack->top = -1;   // EMPTY

    char byte = get_next_byte(get_next_byte_argument);
    // As long as we haven't reached the end of the file, keep getting the next character
    while (byte != EOF)
    {
        int char_cap = 200; // Assume a line will have no more than 200 characters
        int char_count = 0; // Counter for the number of characters in one line
        
        char* line = (char *)checked_malloc(sizeof(char)*char_cap); // Free this after command is parsed
        while (byte != '\n')
        {
            // Doesn't matter what the byte is, just put it in the line
            line[char_count] = byte;
            char_count++;
            // If a line is longer than we expect, update its capacity and reallocate.
            if (char_count >= char_cap)
            {
                char_cap *= 2;
                line = checked_realloc(line, char_cap);
            }
            byte = get_next_byte(get_next_byte_argument);
        }
        // Get rid of the newline still there
        byte = get_next_byte(get_next_byte_argument);
        if (byte == EOF)
            break;
        line[char_count] = '\n';
        char_count++;
        // // Testing: print the line
        // printf("Next line:\n");
        // for (i = 0; i < char_count; i++)
        //     printf("char %d = %c\n", i, line[i]);


        // At this point, all of the bytes in one line are in this "line" array.
        // Now we have to parse them into separate commands, from left to right.
        // THEN, once they're "commanndified", then we can order them on priority.
/////////////////////////////////////////////////////////////////////////////////
        // 1. Turn this array of chars into an array of TOKENS.
        token_stream* ts;
        //printf("Calling tokenize now\n");
        ts = tokenize(line, char_count); // TOKENIZE!!!!!!!!!!
        //printf("Success!\n");
        // printf("Testing: Print the tokenarray\n\n");
        // for (i = 0; i < ts->size; i++)
        // {
        //     if (ts->tokarray[i].type == WORD || ts->tokarray[i].type == LOGICAND || 
        //         ts->tokarray[i].type == LOGICOR)
        //             printf("%s\n", ts->tokarray[i].data.word);
        //     else if (ts->tokarray[i].type == NEWLINE)
        //         printf("newline\n");
        //     else
        //         printf("%c\n", ts->tokarray[i].data.symbol);
        // }
        // printf("Calling parser now\n");
        command_t rootcommand = parse(ts, cmdstack, opstack); // PARSIFY!!!!
        // printf("Parser exited!\n");
        if (rootcommand->status == 1)
        {
            if (stream_index >= commandcap)
            {
                commandcap *= 2;
                stream->carray = checked_realloc(stream->carray, commandcap);
            }
            
            // if (waiting_index > 0) // If there are commands waiting, parse them all!
            // {
            //     // Make sure indices are legit
            //     int d = 0;
            //     for (; d < waiting_index; d++)
            //     {

            //     }
            //     waiting_index = 0;
            // }
            stream->carray[stream_index] = rootcommand;
            stream_index++;
        }
        // else
        // {
        //         if (stream_index >= commandcap)
        //         {
        //             commandcap *= 2;
        //             stream->waitingzone = checked_realloc(stream->waitingzone, commandcap);
        //         }
        //         stream->waitingzone[waiting_index] = rootcommand;
        //         waiting_index++;
        // }
        
    }
    stream->timesread = 0;
    stream->numcmds = stream_index;
    return stream;
}



command_t read_command_stream (command_stream_t s)
{
    /* FIXME: Replace this with your implementation too.  */
    // We'll have an array of command_t, and this function will return 
    // the next command to be printed (in the tree like format).
    
    // Probably going to need to use recursion for subshells, but we'll see...
    if (s->timesread >= s->numcmds)
        return 0;
    
    command_t retvalue = s->carray[s->timesread];
    s->timesread++;
    return retvalue;
}
