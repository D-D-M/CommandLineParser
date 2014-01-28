// UCLA CS 111 Lab 1 command internals

enum command_type
{
  // Reorder the commands by priority, with 0 being lowest priority
  // Might not be necessary but do it just in case
  SIMPLE_COMMAND,      // 0:   a simple command, lowest priority
  SEQUENCE_COMMAND,    // 1:   A ; B
  AND_COMMAND,         // 2:   A && B
  OR_COMMAND,          // 3:   A || B
  PIPE_COMMAND,        // 4:   A | B
  SUBSHELL_COMMAND,    // 6:   ( A )
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
