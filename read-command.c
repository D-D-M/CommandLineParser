// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <error.h>
// Add your own include directives
#include <stdlib.h> // malloc the command stream
#include <stdio.h> // end of file
#include <string.h>
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

// void free_token_stream(token_stream* t)
// {
//     // Need to free the tokarray
//     // Need to free each individual token, depending on size
    
//     int i;
//     printf("Beginning free.\n");
//     for (i = 0; i < t->size; i++)
//     {
//         enum token_type type = t->tokarray[i].type;
//         if (type == WORD || type == LOGICOR || type == LOGICAND)
//         {
//             if (t->tokarray[i].data.word)
//             {
//                 printf("Freeing token %d data.word\n",i);
//                 free(t->tokarray[i].data.word);
//                 t->tokarray[i].data.word = NULL;
//             }
//         }
//         else // if (t->tokarray[i].data.symbol) // If it's a symbol < >
//         {
//             // We don't need to free anything
//             // if (t->tokarray[i].data.symbol)
//             // {
//             //     printf("Freeing token %d data.symbol\n",i);
//             //     free(t->tokarray[i].data.symbol);
//             //     t->tokarray[i].data.symbol = NULL;
//             // }
//         }
//     }

//     if (t->tokarray)
//     {
//         printf("Freeing the tokarray.\n");
//         free(t->tokarray);
//         t->tokarray = NULL;
//     }
//     free(t);
//     t = NULL;
    
//     return;
// }

// If it succeeds, this function accepts a char array and returns a token_stream
// If it fails (because of bad syntax), this function will return NULL.
void tokenize(const char* line, const int linesize, token_stream* ts)
{
    // int sizeguess = 50;
    // int sizeguess = linesize;
    // if (ts != NULL)
    // {
    //     free(ts);
    //     ts = NULL;
    // }
    
    // int ts->size = 0;
    int nt = 0; // number of tokens
    int c = 0; // which char in line[c]
    int w = 0; // which word
    // cat < a || ls foo > b
    for (; c < linesize; )
    {
        if (is_word(&line[c]))
        {
            w = 0;
            int charscap = 15;
            char* word = (char*)checked_malloc(sizeof(char)*charscap);
            while (is_word(&line[c]))
            {
                word[w] = line[c];
                w++; c++;
                // // printf("%c\n", line);
                if (w >= charscap)
                {
                    charscap *= 2;
                    word = checked_realloc(word, charscap);
                }
            }
            word[w] = '\0'; // null terminator
            // // printf("%s\n",word);
            // Put that word in the tokenarray, update token
            // // printf("ts->size %d\n", ts->size);
            ts->tokarray[ts->size].type = WORD;
            // ts->tokarray[ts->size].data.word = word;
            strcpy(ts->tokarray[ts->size].data, word);
            free(word);
            ts->size++;
        }
        else if (line[c] == '#')
            c = linesize; // Comment means we can stop
        else if (line[c] == '<') // IO
        {
            if (c+2 < linesize) // to make sure we don't go inspecting null pointers
            {
                if (line[c+1] == '<' && line[c+2] == '<')
                {
                    fprintf(stderr,"Invalid syntax\n");
                    exit(1);
                }
            }
            ts->tokarray[ts->size].type = I;
            // a[ts->size].data.symbol = line[c];
            strcpy(ts->tokarray[ts->size].data, "<");
            // a[ts->size].data.word = &line[c];
            c++; ts->size++;
        }
        else if (line[c] == '>')
        {
            if (c+2 < linesize) // to make sure we don't go inspecting null pointers
            {
                if (line[c+1] == '>' && line[c+2] == '>')
                {
                    fprintf(stderr,"Invalid syntax\n");
                    exit(1);
                }
            }
            ts->tokarray[ts->size].type = O;
            // a[ts->size].data.symbol = line[c];
            strcpy(ts->tokarray[ts->size].data, ">");
            c++; ts->size++;
        }
        else if (line[c] == '|')
        {
            if (c+2 < linesize) // to make sure we don't go inspecting null pointers
            {
                if (line[c+1] == '|' && line[c+2] == '|')
                {
                    fprintf(stderr,"Invalid syntax\n");
                    exit(1);
                }
            }
            if (line[c+1] == '|') // if the next one is also a pipe, we've got OR
            {
                ts->tokarray[ts->size].type = LOGICOR;
                // a[ts->size].data.word = "||\0";
                strcpy(ts->tokarray[ts->size].data, "||");
                c += 2; 
                ts->size++;
            }
            else // otherwise, we have a pipe
            {
                ts->tokarray[ts->size].type = PIPE;
                strcpy(ts->tokarray[ts->size].data, "|");
                c++; ts->size++;
            }
        }
        else if (line[c] == '&')
        {
            if (c+2 < linesize) // to make sure we don't go inspecting null pointers
            {
                if (line[c+1] == '&' && line[c+2] == '&')
                {
                    fprintf(stderr,"Invalid syntax\n");
                    exit(1);
                }
            }
            if (line[c+1] == '&')
            {
                ts->tokarray[ts->size].type = LOGICAND;
                strcpy(ts->tokarray[ts->size].data, "&&");
                c += 2;
                ts->size++;
            }
            else
            {
                // Might want to consider:
                // return;
                // ^here instead, so that the command stream can be set to invalid
                exit(1); // Invalid syntax, do not support
            }
        }
        else if (line[c] == ';')
        {
            ts->tokarray[ts->size].type = SEMICOLON;
            strcpy(ts->tokarray[ts->size].data, ";");
            c++; ts->size++;
        }
        else if (line[c] == '(' || line[c] == ')')
        {   
            ts->tokarray[ts->size].type = SUBSHELL;
            strcpy(ts->tokarray[ts->size].data, &line[c]);
            c++; ts->size++;
        }
        else if (line[c] == '\n')
        {
            ts->tokarray[ts->size].type = NEWLINE;
            strcpy(ts->tokarray[ts->size].data, "\n");
            c++; ts->size++;
        }
        else if (line[c] == ' ' || line[c] == '\t')
        {
            c++;
        }
        else
        {
            fprintf(stderr,"Invalid syntax, unsupported token %c\n", line[c]);
            exit(1);
        }
    }

    if (ts->tokarray[0].type == LOGICAND || 
        ts->tokarray[0].type == LOGICOR ||
        ts->tokarray[0].type == PIPE ||
        ts->tokarray[0].type == SEMICOLON ||
        ts->tokarray[0].type == I ||
        ts->tokarray[0].type == O )
    {
        fprintf(stderr,"Invalid syntax, line cannot begin with %c \n",*ts->tokarray[0].data);
        exit(1);
    }

    return;
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
        // case WORD:
        // case SUBSHELL:
        // case COMMENT:
        // case I:
        // case O:
        // case NEWLINE:
        case INITIAL:
            return -1;
        default:
            return 0;
    }
}

int cmd_priority(const struct command* P)
{
    switch(P->type)
    {
        case SEQUENCE_COMMAND:
            return 0;
        case AND_COMMAND:
        case OR_COMMAND:
            return 1;
        case PIPE_COMMAND:
            return 2;
        case SIMPLE_COMMAND:
            return 3;
        case SUBSHELL_COMMAND:
            return 4;
        default:
            return -1;
    }
}
// Stack function - PUSH
void push_cmd(cmd_stack_struct *stack, struct command content)
{
    if (stack->top >= stack->capacity)
    {
        stack->capacity *= 2;
        stack = checked_realloc(stack, stack->capacity);
    }
    // stack->con[stack->top] = (struct command)checked_malloc(sizeof(struct command));
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
// Stack function - PUSH
void push_tok(tok_stack_struct *stack, token content)
{
    if (stack->top >= stack->capacity)
    {
        stack->capacity *= 2;
        stack = checked_realloc(stack, stack->capacity);
    }
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

void free_cmd(struct command* c)
{
    if (c->input)
    {
        free(c->input);
        c->input = NULL;
    }
    if (c->output)
    {
        free(c->output);
        c->output = NULL;
    }
    if (c->type == AND_COMMAND || c->type == SEQUENCE_COMMAND || c->type == OR_COMMAND 
        || c->type == PIPE_COMMAND)
    {
        free(c->u.command[0]);
        c->u.command[0] = NULL;
        free(c->u.command[1]);
        c->u.command[1] = NULL;
    }
    else if (c->type == SIMPLE_COMMAND)
    {
        int m = 0;
        while (c->u.word[m] != NULL)
        {
            free(c->u.word[m]);
            c->u.word[m] = NULL;
        }
        free(c->u.word);
        c->u.word = NULL;
    }
    else // Recursively free subshell commands
    {
        free_cmd(c->u.subshell_command);
    }
    free(c);
    return;
}
/*************************
*PARSING HELPER FUNCTIONS*
*************************/
void parse_input(struct command* cmd, const token_stream* ts, cmd_stack_struct* cmdstack, tok_stack_struct* opstack, int* ti)
{
    token* tarray = ts->tokarray;
    if (tarray[*ti].type == WORD)
    {
        // Furthermore, if the NEXT next token is >
        if (tarray[*ti+1].type == O)
        {
            // cmd->input = tarray[*ti].data; // For sure we have input
            cmd->input = checked_malloc(sizeof(char*));
            strcpy(cmd->input, tarray[*ti].data);
            free(tarray[*ti].data);
            tarray[*ti].data = NULL;
            *ti+=2;
            if (tarray[*ti].type == WORD)
            {
                cmd->output = checked_malloc(sizeof(char*));
                strcpy(cmd->output, tarray[*ti].data);
                free(tarray[*ti].data);
                tarray[*ti].data = NULL;
                cmd->status = 1;
                *ti++;
                return;
            }
            else
            {
                cmd->status = -1;
                return;
            }
        }
        else // THE ELSE IS WHAT WORKED BEFORE
        {
            cmd->input = checked_malloc(sizeof(char*));
            strcpy(cmd->input, tarray[*ti].data);
            free(tarray[*ti].data);
            tarray[*ti].data = NULL;
            cmd->output = NULL;
            cmd->status = 1; // complete command
            *ti++;
            return;
        }
    }
    else // This means that < was the last character on a line, resulting in an error
    {
        cmd->status = -1;
        fprintf(stderr,"Invalid syntax\n");//, line cannot end with < \n");
        exit(1);
    }
    return;
}
void parse_output(struct command* cmd, const token_stream* ts, cmd_stack_struct* cmdstack, tok_stack_struct* opstack, int* ti)
{
    token* tarray = ts->tokarray;
    if (tarray[*ti].type == WORD)
    {
        cmd->input = NULL;
        cmd->output = checked_malloc(sizeof(char*));
        strcpy(cmd->output, tarray[*ti].data);
        free(tarray[*ti].data);
        tarray[*ti].data = NULL;
        cmd->status = 1;
        *ti++;
        return;
    }
    else
    {
        fprintf(stderr,"Invalid syntax\n");
        exit(1);
    }
    return;
}
/*******
* JOIN *
********/
// This function takes a 'waiting' LHS command and joins it with a RHS command
struct command* join(struct command* LHS, struct command* RHS)
// void join(struct command* LHS, struct command* RHS)
{
    // How we join the commands depends on the precedence of the two commands
    // Precedence function takes tokens
    // c_priority function takes commands

    // Move LHS down the right side of the tree until 1 before the NULL value
    int i = 0;

    struct command* LROOT = LHS; // marker for the root of the tree so we don't lose it
    int count = 0;
    while (LHS->u.command[1] != NULL) // Find the rightmost NULL node, usually just LHS
    {
        LHS = LHS->u.command[1];
        count++;
    }

    // If it's an actual command
    if (RHS->type == AND_COMMAND || RHS->type == OR_COMMAND || RHS->type == SEQUENCE_COMMAND 
        || RHS->type == PIPE_COMMAND)
    {
        if (cmd_priority(LHS) <= cmd_priority(RHS))
        {
            LHS->u.command[1] = RHS->u.command[0];
            RHS->u.command[0] = LHS;
            // Set the status to 1 for all nodes between LROOT and LHS
            // LHS->status = 1;
            LHS = LROOT;
            if (count == 0)
            {
                LHS->status = 1;
            }
            else
            {
                for (i = 0; i < count; i++)
                {
                    LHS->status = 1;
                    LHS = LHS->u.command[1];
                }
            }

            return RHS;
        }
        else
        {
            LHS->u.command[1] = RHS;
            // Set the status to 1 for all nodes between LROOT and LHS
            // LHS->status = 1;
            LHS = LROOT;
            for (i = 0; i < count; i++)
            {
                LHS->status = 1;
                LHS = LHS->u.command[1];
            }
            // return LHS;
            return LROOT;
        }
    }
    // Else if it's just a simple command
    else if (RHS->type == SIMPLE_COMMAND)
    {
        LHS->u.command[1] = RHS;
            // Set the status to 1 for all nodes between LROOT and LHS
            // LHS->status = 1;
            LHS = LROOT;
            for (i = 0; i < count; i++)
            {
                LHS->status = 1;
                LHS = LHS->u.command[1];
            }
        // return LHS;
        return LROOT;
    }
    else // SUBSHELL COMMAND!
    {
        printf("Subshell command!\n");
        return LHS;
    }
}

/*********
 * PARSE *
 *********/
// Because any command can continue onto the next line, and we're only reading one line at a time,
// we're going to need access to the command stack and the operator stack 
// OUTSIDE of this function, OUTSIDE of even the while !EOF loop

void parse(token_stream *ts, cmd_stack_struct* cmdstack, tok_stack_struct* opstack, command_t root)
{   
    // Simplify
    int maxsize = ts->size;
    token* tarray = ts->tokarray;

    int ti;                     // token iterator 

    // Make sure that we pop off all newlines if they're the only item
    // on top of the opstack when we run parse for a new line
    if (opstack->top == 0) // || topt == SEMICOLON)
    {
        enum token_type topt = opstack->con[0].type;
        while (topt == NEWLINE && opstack->top > -1)
        {
            token removed = pop_tok(opstack);
            topt = opstack->con[0].type;
        }
    }

    // Iterate through the token array
    for(ti = 0; ti < maxsize; ti++)
    {
        struct command* cmd = (struct command*)checked_malloc(sizeof(struct command));
        // cmd->status = 0;
        //
        if (is_simple(&tarray[ti]))
        {
            int z = 0;
            // Initialize command
            cmd->u.word = (char**)checked_malloc(sizeof(char*)*maxsize);
            while(tarray[ti].type == WORD)
            {
                // Word isn't being set properly
                // cmd->u.word[z] = tarray[ti].data.word;
                cmd->u.word[z] = checked_malloc(sizeof(char*));
                strcpy(cmd->u.word[z], tarray[ti].data);
                free(tarray[ti].data);
                tarray[ti].data = NULL;
                ti++; z++;
            }
            if (tarray[ti].type == I)           // INPUT < SYMBOL
            {
                ti++;
                parse_input(cmd, ts, cmdstack, opstack, &ti);
            }
            else if (tarray[ti].type == O)      // OUTPUT > SYMBOL
            {
                ti++;
                parse_output(cmd, ts, cmdstack, opstack, &ti);
            }
            else if (tarray[ti].type == NEWLINE || tarray[ti].type == SEMICOLON)
            {
                cmd->status = 1;
                ti--;
            }
            else if (tarray[ti].type == LOGICOR || tarray[ti].type == LOGICAND || tarray[ti].type == PIPE)
            {
                cmd->status = 1; // Might need to comment this out, look at the idea you wrote
                ti--;
            }
            else
                ti--;
            cmd->type = SIMPLE_COMMAND;
            // Now we either have a simple command with a status
            push_cmd(cmdstack, *cmd);
            // printf("Stack push_cmd!\n");
            // Free that command
            //free_cmd(cmd);
        }
        // Sequence command needs to know if there's anything after it
        else // if (is_op(&ts->tokarray[ti]))     // is an operator: &&, ||, |, ;
        {
            // If the opstack is empty, just push this operator onto it, badaboom
            if (opstack->top == -1)
            {
                // printf("Pushing op at position %d !\n", ti);
                push_tok(opstack, tarray[ti]);
            }
            // If the precedence of the operator on the opstack is greater than the precedence
            // of the token you've just run into, you need to start linking commands up.
            // else if (precedence(opstack->con[opstack->top]) >= precedence(tarray[ti]) || ti >= maxsize)
            else if (precedence(opstack->con[opstack->top]) >= precedence(tarray[ti]))
            {
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
                    // case WORD:
                    // case SUBSHELL:
                    // case COMMENT:
                    // case I:
                    // case O:
                    //     // printf("Parse returning NULL now\n");
                    //     return NULL; 
                    default:
                        exit(1);
                }
                cmd->input = NULL;
                cmd->output = NULL;
                cmd->status = 0; // Command status should be "waiting" until we know that we have 
                                // two commands on the stack, ready to be joined
                // How many times we're going to pop the stack
                // int numpops = determine_stackpops(cmdstack, opstack);

                if (cmdstack->top != -1)
                // if (numpops >= 1)
                {
                    // Set the value of the command
                    *R = pop_cmd(cmdstack);
                    token OP = pop_tok(opstack); // opstack is now size-1
                    if (cmdstack->top != -1)
                    {
                        // If there's nothing left in the opstack, do what we normally do
                        // if (opstack->top == -1) 
                        if (cmdstack->top > opstack->top)
                        {
                            *L = pop_cmd(cmdstack);
                            cmd->u.command[0] = L;
                            cmd->u.command[1] = R;
                            // if (L->status == 1 && R->status == 1)
                            // {
                            //     cmd->status = 1; // Not waiting anymore
                            // }
                            cmd->status = L->status && R->status; // Not waiting anymore
                            // cmd->status = 1;

                        }
                        else //if (precedence(OP) > precedence(opstack->con[opstack->top]))
                        {
                            cmd->u.command[0] = R; // still waiting
                        }
                    } 
                    else // There are no commands to pair cmd with, i.e. cmdstack is empty
                    {
                        cmd->u.command[0] = R; // still waiting
                    }
                }
                else // The stack is empty
                {
                    // printf("operator with no subcommands\n");
                    // cmd->status = -1; // error
                }

                push_cmd(cmdstack, *cmd);
                //free_cmd(cmd); // Free the command now that you've pushed its value onto the cmdstack
                
                // printf("tarray[%d] type is %d\n", ti, tarray[ti].type);
                ti--;
                                
                // if (tarray[ti].type == NEWLINE || tarray[ti].type == SEMICOLON)
                // {
                //     // while (cmdstack->top > 0)
                //     // {

                //     // }
                // }
                // else
                //     ti--;   // Decrement - This makes sense, don't make Eskild explain it to you again.                                  
                
            }
            else
            {
                // printf("Pushing op at position %d !\n", ti); 
                push_tok(opstack, tarray[ti]);
            }
        }
    }

    *root = pop_cmd(cmdstack);
    // Test what the value of root is before we return it
    // printf("root type is %d\n",root->type);
    // printf("root status is %d\n", root->status);
    // if (root->)
    // // printf("Is root null? Check: %p\n", root);
    // return root;
}

void initialize_cmd(struct command* c)
{
    c->type = NOT_A_COMMAND_YET;
    c->status = 0;
    c->input = 0;
    c->output = 0;
    // c->u.word = 0;
}

// This time, our implementation will be to read one line at a time
command_stream_t make_command_stream (int (*get_next_byte) (void *),
              void *get_next_byte_argument)
{
    int i = 0;
    

    // // Initialize TOKEN STREAM
    // token_stream* ts = (token_stream*)checked_malloc(sizeof(token_stream));
    // int guess_num_tokens = 64;
    // ts->tokarray = (token*)checked_malloc(sizeof(token)*guess_num_tokens);
    // ts = 0;
    // ts->size = 0;


    // Initialize COMMAND STREAM 
    command_stream_t cs = (command_stream_t)checked_malloc(sizeof(struct command_stream));
     // Assume a file will have no more than 32 root commands
    // int cmd_count = 0; // Counter for the number of commands in one file
    int commandcap = 64;
    int cmd_it = 0; // iterator for finding the next command in a line
    cs->carray = (command_t)checked_malloc(sizeof(struct command)*commandcap);
    for (i = 0; i < commandcap; i++)
    { // Initialize each command
        initialize_cmd(&cs->carray[i]);
    }
    cs->stream_index = 0;
    cs->waitingzone = checked_malloc(sizeof(struct command));
    initialize_cmd(cs->waitingzone); // might need to change this line
    cs->wi = 0;
    cs->timesread = 0;
    // cs->numcmds = 0;
    cs->valid_syntax = 0; // -1 if bad, 0 if good
    cs->num_parentheses = 0;

    
    // Initialize COMMAND STACK
    cmd_stack_struct* cmdstack = (cmd_stack_struct*)checked_malloc(sizeof(cmd_stack_struct));
    cmdstack->capacity = 128;
    cmdstack->con = (struct command*)checked_malloc(sizeof(struct command)*(cmdstack->capacity));
    cmdstack->top = -1;  // EMPTY


    // Initialize OPERATOR STACK
    tok_stack_struct* opstack = (tok_stack_struct*)checked_malloc(sizeof(tok_stack_struct));
    opstack->capacity = 128;
    opstack->con = (token*)checked_malloc(sizeof(token)*(opstack->capacity));
    opstack->top = -1;   // EMPTY


    
    char byte = get_next_byte(get_next_byte_argument);
    // As long as we haven't reached the end of the file, keep getting the next character
    while (byte != EOF)
    {
        if (byte == '\n')
        {
            byte = get_next_byte(get_next_byte_argument);
        }
        else
        {
            int char_cap = 200; // Assume a line will have no more than 200 characters
            int char_count = 0; // Counter for the number of characters in one line
            
            char* line = (char *)checked_malloc(sizeof(char)*char_cap); // FREE AT LAST
            for (i = 0; i < char_cap; i++) // Initialize line, might not be necessary
                line[i] = 0;

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
            // if (byte == EOF)
            //     break;
            line[char_count] = '\n';
            char_count++;
            // // Testing: print the line
            // // printf("Next line:\n");
            // for (i = 0; i < char_count; i++)
            //     // printf("char %d = %c\n", i, line[i]);


            // At this point, all of the bytes in one line are in this "line" array.
            // Now we have to parse them into separate commands, from left to right.
            // THEN, once they're "commanndified", then we can order them on priority.
    
            // 1. Turn this array of chars into an array of TOKENS.

            // Initialize TOKEN STREAM
            token_stream* ts = (token_stream*)checked_malloc(sizeof(token_stream));
            int guess_num_tokens = 64;
            ts->tokarray = (token*)checked_malloc(sizeof(token)*guess_num_tokens);
            // Might need to initialize each individual token here
            for(i = 0; i < guess_num_tokens; i++)
            {
                // char* c = "initial";
                // *(ts->tokarray[i].data) = *c;
                // ts->tokarray[i].data = c;
                ts->tokarray[i].data = checked_malloc(sizeof(char*));
                ts->tokarray[i].type = INITIAL;
            }
            ts->size = 0;            

            tokenize(line, char_count, ts);
            //// printf("Success!\n");
            // // printf("Testing: Print the tokenarray\n\n");
            // for (i = 0; i < ts->size; i++)
            // {
            //     if (ts->tokarray[i].type == WORD || ts->tokarray[i].type == LOGICAND || 
            //         ts->tokarray[i].type == LOGICOR)
            //             // printf("%s\n", ts->tokarray[i].data.word);
            //     else if (ts->tokarray[i].type == NEWLINE)
            //         // printf("newline\n");
            //     else
            //         // printf("%c\n", ts->tokarray[i].data.symbol);
            // }
            // // printf("Calling parser now\n");
            if (ts->size > 0)
            {
                struct command* p_rootcommand = (command_t)checked_malloc(sizeof(struct command));
                parse(ts, cmdstack, opstack, p_rootcommand); // PARSIFY!!!!
                
                // Now that we have the rootcommand*, we need to copy its values into an actual command
                // and put that command in carray if it's complete.
                // struct command rootcommand = *p_rootcommand;

                // Possibly free the token stream here:            
                // free(ts->tokarray);
                // free(ts);

                // // printf("Parser exited!\n");

                if (p_rootcommand->status == 1)
                {
                    if (cs->stream_index >= commandcap)
                    {
                        commandcap *= 2;
                        cs->carray = checked_realloc(cs->carray, commandcap);
                    }
                    
                    if (cs->wi) // If there is a command waiting, join it!
                    {
                        p_rootcommand = join(cs->waitingzone, p_rootcommand);
                        cs->wi = 0;
                    }
                    cs->carray[cs->stream_index] = *p_rootcommand;
                    cs->stream_index++;
                }
                else // INCOMPLETE COMMAND: we should either stage it for waiting, or join it with
                     // the thing that's currently waiting
                {
                    if (cs->wi) // If we're already waiting on something, join those two things
                    {
                        // printf("Joining\n");
                        cs->waitingzone = join(cs->waitingzone, p_rootcommand);
                        // join(&cs->waitingzone, &rootcommand);
                        cs->wi = 1;
                    }
                    else // There's nothing to be waited on.
                    {
                        // printf("Add to waiting zone\n");
                        cs->waitingzone = p_rootcommand;
                        cs->wi = 1;
                    }
                }
                // for (i = 0; i < char_count; i+d+)
                // {
                //     free(line[i]);
                //     line[i] = NULL;
                // }
                free(line);
                // free_token_stream(ts);
            }
        }

    }
    // If our waitingzone is a SEQUENCE command with a null command[1] value,
    // then we're not actually waiting for anything, and we can add this command
    // to the next slot in carray.
    //
    if (cs->waitingzone->type == SEQUENCE_COMMAND)
    {
        // Update the waiting zone to be the LHS
        cs->waitingzone = cs->waitingzone->u.command[0];
        // Add it to the next slot in the array
        cs->carray[cs->stream_index] = *cs->waitingzone;
        cs->stream_index++;
    }
    // stream->timesread = 0;
    // cs->numcmds = stream_index;
    // free(stream->waitingzone);
    return cs;
}



command_t read_command_stream (command_stream_t s)
{
    /* FIXME: Replace this with your implementation too.  */
    // We'll have an array of command_t, and this function will return 
    // the next command to be printed (in the tree like format).

    // Probably going to need to use recursion for subshells, but we'll see...
    // if the command in cs[i-1] IS a command, then free it and set it to NULL
    if (s->timesread > 0)
    {
        if (s->carray[s->timesread - 1].type != NOT_A_COMMAND_YET)
        {
            //free_cmd(&s->carray[s->timesread - 1]);
            // s->carray[s->timesread-1] = NULL;
        }
    }

    if (s->timesread >= s->stream_index)
        return 0;
    
    command_t retvalue = &s->carray[s->timesread];
    // free(s->carray[s->timesread]);
    s->timesread++;
    // Free all of the memory we're using in the command stream HERE
    // free(s->waitingzone);
    return retvalue;
}
