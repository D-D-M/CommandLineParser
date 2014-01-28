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
    int numcmds; // number of commands in a stream
    // int* hp; // highest priority in each line?? hp[0] = 5; hp[1] = 3;

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

/*
// This function determines if a char c can be part of a command
int is_cmd(const char* c)
{
    // Ignoring redirection (possibly pipes and subshells too) this time
    if (*c == '|' || *c == '&' || *c == '(' || *c == ')' || *c == ';')
        return 1; // TRUE, this char is a command
    else
        return 0; // FALSE, this char is NOT a command
}
*/
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

int precedence(const token *t)
{
    switch(t->type)
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

// pf is the postfix array of commands that you're trying to link
// pi is the current index of the postfix[i] array you're looking at.
/*
struct command link_cmd(struct command* pf, int pi, command_t LHS, command_t RHS)
{
    struct command
}
// Returns a full populated command struct 
struct command make_simple_cmd(token *con, int b, int e)
{
    struct command cmd; // Might need to malloc this, and we'll know for sure
                        // when testing causes a segfault or the command just
                        // has no data whatsoever. COULD BE FINE THOUGH.
    int i;

    if (is_simple(&con[b]))
    {
        cmd.type = SIMPLE_COMMAND;
    }   
    else if (is_op(&con[b]))
    {
        switch(con->type)
        {
            case PIPE:
              cmd.type = PIPE_COMMAND;
            case LOGICAND:
              cmd.type = AND_COMMAND;
            case LOGICOR:
              cmd.type = OR_COMMAND;
            case SEMICOLON:
              cmd.type = SEQUENCE_COMMAND;
            case WORD:
            case SUBSHELL:
            case COMMENT:
            case IO:
            case NEWLINE:
              exit(1);
        }
    }

    
    return cmd;
}
*/

/* **********************
 * COMMAND STACK STRUCT *
 ************************/ 

// Stack struct with an array of command structs
typedef struct 
{
    struct command* con;    // SEE command-internals.h
    int top;        // -1 = EMPTY
    int size;
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
    int size;
    int *holder;    // an array of ints that holds the place of the operator 
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

command_t parse(token_stream *ts, cmd_stack_struct* cmdstack, tok_stack_struct* opstack)
{   
    // Simplify
    int maxsize = ts->size;
    token* tarray = ts->tokarray;
    // This function will return 'root'
    command_t root = (command_t)checked_malloc(sizeof(struct command)*maxsize);

    int ti;                     // token iterator 
    // tok_stack_struct opstack;   // operator stack, used for &&, ||, | and ; only
    // tok_stack_struct stack;     // stack with simple commands, simple commands with I/O
    // cmd_stack_struct cmdstack;  // command stack (filling in the command details    

    // Initial stacks with ts->size possible slots  
    // NOTE: no stack, array, etc can have more elements than ts->size.
    // opstack.con = (token*)checked_malloc(sizeof(token)*(ts->size));
    // opstack.top = -1;   // EMPTY
    // stack.con = (token*)checked_malloc(sizeof(token)*(ts->size));
    // stack.top = -1;     // EMPTY
    // cmdstack.con = (struct command*)checked_malloc(sizeof(struct command)*(ts->size));
    // cmdstack.top = -1;  // EMPTY
    
    int pi = 0;         // postfix iterator 
    // struct command *postfix = (struct command*)checked_malloc(sizeof(struct command)*(ts->size));

    // token t = &ts->tokarray;
    // Iterate through the token array
    for(ti = 0; ti < maxsize; ti++)
    {
        //
        // PSEUDOCODE begins here
        //
        struct command cmd; // This is the command that will be added to the cmdstack after the iteration

        if (is_simple(&tarray[ti]))         // FISHY, will check later.
        {
            int z = 0;
            cmd.u.word = (char**)checked_malloc(sizeof(char*)*maxsize);
            while(tarray[ti].type == WORD)
            {
                // Word isn't being set properly
                cmd.u.word[z] = tarray[ti].data.word;
                ti++; z++;
            }
            if (tarray[ti].type == I)
            {
                if (tarray[ti+1].type == WORD)
                {
                    cmd.input = tarray[ti+1].data.word;
                    cmd.output = NULL;
                    ti++;
                    cmd.status = 1; // complete command
                }
                else
                {
                    if (tarray[ti+1].type == NEWLINE)
                        cmd.status = 0; // incomplete command
                    else
                        cmd.status = -1; // error (we think)
                }
            }
            else if (tarray[ti].type == O)
            {
                if (tarray[ti+1].type == WORD)
                {
                    cmd.input = NULL;
                    cmd.output = tarray[ti+1].data.word;
                    ti++;
                    cmd.status = 1; // complete command
                }
                else
                {
                    if (tarray[ti+1].type == NEWLINE)
                        cmd.status = 0; // incomplete command
                    else
                        cmd.status = -1; // error (we think)
                }
            }
            cmd.type = SIMPLE_COMMAND;
            // Now we either have a simple command with a status
            push_cmd(cmdstack, cmd);
            printf("Stack push!\n");


            
        }
    /*
        else // if (is_op(&ts->tokarray[ti]))     // is an operator: &&, ||, |, ;
        {
            // If the opstack is empty, just push this operator onto it, badaboom
            if (opstack.top == -1)
            {
                push_tok(&opstack, ts->tokarray[ti]);
            }
            // If the precedence of the operator on the opstack is greater than the precedence
            // of the token you've just run into, you need to start linking commands up.
            else if (precedence(&opstack.con[opstack.top]) >=        // check the = 
                precedence(&ts->tokarray[ti]))
            {
                // if command is simple
                postfix[pi] = make_simple_cmd(stack.con, stack.holder[hi-2]+1, stack.holder[hi-1]); 
                pi++;   
                postfix[pi] = make_simple_cmd(stack.con, stack.holder[hi-1]+1, stack.top); // check the holder
                pi++;

                // LHS will be postfix[pi - 1] ?
                // RHS will be postfix[pi - 2] ?
                //postfix[pi] = link_cmd(pop_tok(&opstack), opstack.top+1, opstack.top+1);
                pi++;
            // pop the stack as well
            
            // if we have a semicolon, we need to decrement ti-- to re-do this whole procedure
            // with the opstack shorter by 1                                        
            }
            else
            { 
                push_tok(&opstack, ts->tokarray[ti]);
            }
        }
            stack.holder[hi] = stack.top;   // after each operator, hold its place in the stack 
            hi++;
        */
    }

    return NULL;
}





// This time, our implementation will be to read one line at a time
command_stream_t make_command_stream (int (*get_next_byte) (void *),
              void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
      add auxiliary functions and otherwise modify the source code.
      You can also use external functions defined in the GNU C Library.  */
    int i;
    int strm_index = 0;
    
    // Allocate space for the command stream
    // NOTE: A command_stream_t is a POINTER to a struct command_stream
    command_stream_t stream = (command_stream_t)checked_malloc(sizeof(struct command_stream));
    // Initialize command stream
    size_t commandcap = 100; // Assume a file will have no more than 100 commands
    int cmd_count = 0; // Counter for the number of commands in one file
    int cmd_it = 0; // iterator for finding the next command in a line
    stream->carray = (command_t*)checked_malloc(sizeof(command_t)*commandcap);
    stream->valid_syntax = 1;

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
        // Don't forget that if byte == '#', we should essentially
        // ignore everything
        // in between # and the next newline, because it's just a comment and we don't
        // care about those!
        //

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
        line[char_count] = '\n';
        char_count++;
        // Testing: print the line
        printf("Next line:\n");
        for (i = 0; i < char_count; i++)
            printf("char %d = %c\n", i, line[i]);


        // At this point, all of the bytes in one line are in this "line" array.
        // Now we have to parse them into separate commands, from left to right.
        // THEN, once they're "commanndified", then we can order them on priority.
/////////////////////////////////////////////////////////////////////////////////
        // 1. Turn this array of chars into an array of TOKENS.
        token_stream* ts;
        ts = tokenize(line, char_count);

        printf("Testing: Print the tokenarray\n\n");
        for (i = 0; i < ts->size; i++)
        {
            if (ts->tokarray[i].type == WORD || ts->tokarray[i].type == LOGICAND || 
                ts->tokarray[i].type == LOGICOR)
                    printf("%s\n", ts->tokarray[i].data.word);
            else
                printf("%c\n", ts->tokarray[i].data.symbol);
        }
        command_t rootcommand = parse(ts, cmdstack, opstack);





/////////////////////////////////////////////////////////////////////////////////
/*
 

        if (line_count != 0) // Parse the command if there's a command to parse.
        {
            // First good idea would be to iterate through the array to see what
            // our highest priority command is, and then base all of the other
            // commands off of that one. /BAD

            // enum command_type hp = find_hp(&line[0], line_count, get_next_byte, get_next_byte_argument);
            // fprintf(stdout, "Highest priority is %d\n", hp);
            int pos_next_cmd;
            // Find the position of the next command
            for (; cmd_it < line_count; cmd_it++)
            {
                if (is_cmd(&line[cmd_it])) // Don't forget about pipes
                {
                    if (line[cmd_it] == '|' && line[cmd_it+1] != '|')
                        // if we see a pipe, but not another pipe immediately after it
                        // then do nothing
                        ;
                    else
                    {
                        pos_next_cmd = cmd_it;
                        break;
                    }
                }
            }
            // fprintf(stdout, "Position of next command is %d\n", pos_next_cmd);
            // Read every word up to that point
            int word_cap = 10;
            int letter_cap = 20;
            char** wordarray = (char**)checked_malloc(sizeof(char*) * word_cap);
            
            // We need to read words up until the first non-simple command
            int array_index = 0; // iterator for the wordarray
            i = 0;
            int word_index = 0;
            // Put individual words into the wordarray
            while (i < pos_next_cmd)
            {
                char* word = (char*)checked_malloc(sizeof(char) * letter_cap);
                printf("Begin loop with i = %d\n", i);
                word_index = 0;
                while (line[i] != ' ')
                {
                    word[word_index] = line[i];
                    // printf("letter %d is %c\n", i, word[word_index]);
                    i++;
                    word_index++;
                    if (i >= letter_cap) // Make sure it doesn't grow too big
                    {
                        letter_cap *= 2;
                        word = (char*)checked_realloc(word, letter_cap);
                    }
                }
                word[word_index] = '\0'; // Add null terminator
                i++;
                
                wordarray[array_index] = word;
                fprintf(stdout, "Word #%d is %s\n", array_index, wordarray[array_index]);
                array_index++;
                if (array_index >= word_cap)
                {
                    word_cap *= 2;
                    wordarray = (char**)checked_realloc(wordarray, word_cap);
                }
                // i++;
                printf("End loop with i = %d\n", i);
            }
            
            // TESTING
            for (i = 0; i < array_index; i++)
                fprintf(stdout, "%s\n", wordarray[i]);

            // Now that we have a word array with IO redirections built in,
            // allocate a command, and put it in the next available slot of carray
            command_t nextcmd = (command_t)checked_malloc(sizeof(struct command));
            nextcmd->type = SIMPLE_COMMAND; // might be premature
            // Fill out the structure
            // Case 1a: Search for location of <
            int searchin = 0;
            int inpos = -1; // pretending that there will only be 1 < in a line
            for ( ; searchin < array_index; searchin++)
            {
                if (*wordarray[searchin] == '<')
                {
                    inpos == searchin;
                    break;
                }
            }
            // Case 1b: Search for location of >
            int searchout = 0;
            int outpos = -1; // pretendoutg that there will only be 1 < out a line
            for ( ; searchout < array_index; searchout++)
            {
                if (*wordarray[searchout] == '>')
                {
                    outpos == searchout;
                    break;
                }
            }
            // Case 2: Search for a pipe
            int searchpipe = 0;
            int pipepos = -1; // pretend that there will only be 1 < pipe a line
            for ( ; searchpipe < array_index; searchpipe++)
            {
                if (*wordarray[searchpipe] == '|')
                {
                    pipepos == searchpipe;
                    break;
                }
            }
            if (inpos > 0) // because bad syntax to have '$ < a'
            {
                nextcmd->input = wordarray[inpos+1];
                nextcmd->output = NULL;
                int left = 0;
                for ( ; left < inpos; left++)
                {
                    nextcmd->word[left] = wordarray[left];
                }
                nextcmd->word[inpos] = NULL;
                stream->carray[strm_index] = nextcmd;
            }
            else if (outpos > 0)
            {
                nextcmd->input = NULL;
                nextcmd->out = wordarray[outpos+1];
                int left = 0;
                for (; left < outpos; left++)
                {
                    nextcmd->word[left] = wordarray[left]
                }
                nextcmd->word[outpos] = NULL;
                stream->carray[strm_index] = nextcmd;
            }
            

            // see what's in posnextcmd (if there is anything there at all) 
            // and put IT into a command while we're at it


        }
        // Parse SIMPLE commands first, because that takes care of the IO stuff
        // THEN, we can start ordering these simple commands based on what
        
        // Exiting this loop means that byte = newline, and we can begin with:
        // 1. If line_count != 0,
        // 2. Try parsing the command
        // 3. Then put that command into 'stream[cmd_count]'
        // 4. cmd_count++;

        // Testing
        // for (i = 0; i < line_count; i++)
                // fprintf(stdout, "%c\n", line[i]);
        // Reset everything
        line_count = 0;
        // If we haven't done so already, update byte
        byte = get_next_byte(get_next_byte_argument);
    */

    }

    return stream;
    
}



command_t
read_command_stream (command_stream_t s)
{
    /* FIXME: Replace this with your implementation too.  */
    // We'll have an array of command_t, and this function will return 
    // the next command to be printed (in the tree like format).
    
    // Probably going to need to use recursion for subshells, but we'll see...
    error (1, 0, "command reading not yet implemented");
    return 0;
}
