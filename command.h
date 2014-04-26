// UCLA CS 111 Lab 1 command interface
#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <stdbool.h>

typedef struct command *command_t;
typedef struct command_stream *command_stream_t;

typedef struct list_node list_node;
typedef struct list list;
typedef struct graph_node graph_node;
typedef struct queue_node queue_node;
typedef struct dependency_graph dependency_graph;

/* Create a command stream from GETBYTE and ARG.  A reader of
   the command stream will invoke GETBYTE (ARG) to get the next byte.
   GETBYTE will return the next input byte, or a negative number
   (setting errno) on failure.  */
command_stream_t make_command_stream (int (*getbyte) (void *), void *arg);

/* Read a command from STREAM; return it, or NULL on EOF.  If there is
   an error, report the error and exit instead of returning.  */
command_t read_command_stream (command_stream_t stream);

/* Print a command to stdout, for debugging.  */
void print_command (command_t);

/* Execute a command.  Use "time travel" if the flag is set.  */
void execute_command (command_t, bool);

/* Return the exit status of a command, which must have previously
   been executed.  Wait for the command, if it is not already finished.  */
int command_status (command_t);

dependency_graph* create_graph (command_stream_t stream);

void execute_graph (dependency_graph *graph);

#endif
