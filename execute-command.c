// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>
#include <errno.h>	// for open, close

#include <unistd.h>	// for execvp, dup2, close, fork
#include <sys/types.h>	// for open, waitpid
#include <sys/stat.h> 	// for open
#include <fcntl.h>	// for open
#include <stdlib.h>
#include <stdio.h>


/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

// Leave unchanged
int command_status (command_t c)
{
	return c->status;
}

void exe_simple(command_t c)
{
	pid_t proc_id = fork();
	int fd;
	char **argv = c->u.word;	

	if (proc_id == 0 && c->input != NULL)
	{	
		fd = open(c->input, O_RDONLY);	
		dup(fd);
		close(fd);
		execvp(argv[0], argv); 
		
	}
	else if (proc_id == 0 && c->output != NULL)
	{
		fd = open(c->output, O_RDONLY || O_WRONLY || O_CREAT);	// check O_CREAT!!!
		dup(fd);
		close(fd);
		execvp(argv[0], argv);
	}
	else if (proc_id == 0 && c->input == NULL && c->output == NULL)
	{
		execvp(argv[0], argv);
	}
	else // what if proc_id != 0??? 
	{
		c->exe = 0;
		return;
	}
	// this command has been executed 
	c->exe = 1;
	return;
}

void exe_sequence(command_t c)
{
}

void exe_branch(command_t c)
{
	// if command has already been executed no need to continue
	// this might need to be expanded
	if (c->exe == 1)
		return;

	switch(c->type)
	{
	   case SIMPLE_COMMAND:
	   {
		exe_simple(c);
		break;
	   }
	   case SEQUENCE_COMMAND:
	   {
	
	   }
	   case AND_COMMAND:
	   {
		exe_branch(c->u.command[0]);		// execute left branch 
		if (c->u.command[0]->exe == 1)		// if left successful, execute right branch 
			exe_branch(c->u.command[1]);
		else					// if left fails, set exe = 0
		{ 
			c->exe = 0;
			break;
		}
		if (c->u.command[1]->exe == 0)		// if right fails, set exe = 0 
			c->exe = 0;
		break;
	   }
	   case OR_COMMAND:
	   {
		exe_branch(c->u.command[0]);
		if (c->u.command[0]->exe == 1)
			break;
		exe_branch(c->u.command[1]);
		if (c->u.command[1]->exe == 0)
			c->exe = 0;
		break;

	   }
	   case PIPE_COMMAND:
	   {
	   }
	   case SUBSHELL_COMMAND:
	   {
	   } 
	}
}

void execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */

	exe_branch(c);

	//error (1, 0, "command execution not yet implemented");
}
