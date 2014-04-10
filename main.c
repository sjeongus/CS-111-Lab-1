// UCLA CS 111 Lab 1 main program

#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "command.h"
#include "read-command.h"

static char const *program_name;
static char const *script_name;

static void
usage (void)
{
  error (1, 0, "usage: %s [-pt] SCRIPT-FILE", program_name);
}

static int
get_next_byte (void *stream)
{
  return getc (stream);
}

int
main (int argc, char **argv)
{
  stack *cmd_stack = init_stack(5);
  stack *op_stack = init_stack(5);
  command_t cmd1 = malloc(sizeof(struct command));
  command_t cmd2 = malloc(sizeof(struct command));
  command_t op1 = malloc(sizeof(struct command));
  cmd1->type = SIMPLE_COMMAND;
  cmd2->type = SIMPLE_COMMAND;
  char* yolo = "yolo";
  char* swag = "swag";
  cmd1->u.word = malloc(sizeof(char*));
  cmd2->u.word = malloc(sizeof(char*));
  cmd1->u.word[0] = malloc(strlen(yolo)+1);
  cmd2->u.word[0] = malloc(strlen(swag)+1);
  strcpy(cmd1->u.word[0], yolo);
  strcpy(cmd2->u.word[0], swag);
  fprintf(stderr, "%s\n", cmd1->u.word[0]);
  op1->type = PIPE_COMMAND;
  push(cmd_stack, cmd1);
  push(cmd_stack, cmd2);
  push(op_stack, op1);
  handle_operator(AND_COMMAND, cmd_stack, op_stack);
  fprintf(stderr, "%s\n", peek(cmd_stack)->u.command[0]->u.word[0]);
  fprintf(stderr, "%s\n", peek(cmd_stack)->u.command[1]->u.word[0]);

  return 0;

  /*char** toks = tokenize_expression(buffer, &buffer_size);

  printf("%s\n", narr);
  printf("%s\n", buffer);
  unsigned int k;
  for (k = 0; k < 7; k++)
    printf("%s\n", toks[k]);

  return 0;*/

  /*int command_number = 1;
  bool print_tree = false;
  bool time_travel = false;
  program_name = argv[0];

  for (;;)
    switch (getopt (argc, argv, "pt"))
      {
      case 'p': print_tree = true; break;
      case 't': time_travel = true; break;
      default: usage (); break;
      case -1: goto options_exhausted;
      }
 options_exhausted:;

  // There must be exactly one file argument.
  if (optind != argc - 1)
    usage ();

  script_name = argv[optind];
  FILE *script_stream = fopen (script_name, "r");
  if (! script_stream)
    error (1, errno, "%s: cannot open", script_name);
  command_stream_t command_stream =
    make_command_stream (get_next_byte, script_stream);

  command_t last_command = NULL;
  command_t command;
  while ((command = read_command_stream (command_stream)))
    {
      if (print_tree)
	{
	  printf ("# %d\n", command_number++);
	  print_command (command);
	}
      else
	{
	  last_command = command;
	  execute_command (command, time_travel);
	}
    }

  return print_tree || !last_command ? 0 : command_status (last_command);*/
}
