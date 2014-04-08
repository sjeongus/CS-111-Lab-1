// UCLA CS 111 Lab 1 command reading
// header file for testing purposes only

#ifndef _READ_COMMAND_H_
#define _READ_COMMAND_H_

#include "alloc.h"
#include "command.h"
#include "command-internals.h"

typedef struct command_node command_node;
typedef struct command_stream command_stream;
typedef struct stack stack;
typedef enum command_type command_type;

void push (stack *self, command_t command);

command_t pop (stack *self);

command_t peek (stack *self);

stack* init_stack (int max);

bool is_operator (char c);

void buffer_append (char c, char *buffer, int *size, int *max);

void clear_buffer (char *buffer);

bool is_greater_precedence (command_type a, command_type b);

char** tokenize_expression (char* buffer, int *size);

void handle_operator (command_type op, stack *cmd_stack, stack *op_stack);

void handle_command (char **words, stack *cmd_stack, int num_words);

command_node* process_expression (char *buffer);

void append_node (command_node *node, command_stream_t stream);

command_stream_t make_command_stream (int (*get_next_byte) (void *),
         void *get_next_byte_argument);

command_t read_command_stream (command_stream_t s);

#endif
