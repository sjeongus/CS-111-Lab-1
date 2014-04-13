// UCLA CS 111 Lab 1 command reading

#include "read-command.h"
/*
#include "alloc.h"
#include "command.h"
#include "command-internals.h"
*/

#include <stdio.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEFAULT_WORDS 5
#define DEFAULT_STACK_ITEMS 10
#define DEFAULT_BUFFER_SIZE 500
#define SEQUENCE_CHAR ';'
#define PIPE_CHAR '|'
#define OR_SEQ "||"
#define AND_SEQ "&&"

/*
typedef struct command_node command_node;
typedef struct command_stream command_stream;
typedef struct stack stack;
typedef enum command_type command_type;

struct command_node
{
  command_t command;
  command_node *next;
};

struct command_stream
{
  command_node *head;
  command_node *iterator;
  int index;
  int size;
};

struct stack
{
  command_t *commands;
  int size;
  int max_size;
  void (* push)();
  command_t (* pop)();
  command_t (* peek)();
};
*/

// all stack functions working properly
void
push (stack *self, command_t command)
{
  if (self->size == self->max_size) {
    self->max_size *= 2;
    self->commands = checked_realloc(self->commands, self->max_size * sizeof(command_t));
  }

  self->commands[self->size] = command;
  self->size++;
}

command_t
pop (stack *self)
{
  if (self->size > 0) {
    self->size--;
    return self->commands[self->size];
  } else
    return NULL;
}

command_t
peek (stack *self)
{
  if (self->size > 0) {
    int i = self->size - 1;
    return self->commands[i];
  } else
    return NULL;
}

stack*
init_stack (int max)
{
  stack *new = checked_malloc(sizeof(stack));
  new->size = 0;
  new->max_size = max;
  new->commands = checked_malloc(max * sizeof(struct command));
  return new;
}

command_t
new_command (void)
{
  command_t new_cmd = checked_malloc(sizeof(struct command));
  new_cmd->input = NULL;
  new_cmd->output = NULL;
  return new_cmd;
}

// Tested and works!
bool
is_operator (char c)
{
  switch (c)
  {
    case '|':
    case '&':
    case ';':
    case '(':
      return true;
    default:
      return false;
  }
}

// Tested and works!
// safe append for the buffer, reallocs as necessary
// adds a space before each operator for 420 offdarailz tokenization
void
buffer_append (char c, char *buffer, int *size, int *max)
{
  if (*size >= *max) {
    *max *= 2;
    buffer = checked_realloc(buffer, *max);
  }

  if ((is_operator(c) && !is_operator(buffer[(*size)-1]))
       || isalnum(c) && is_operator(buffer[(*size)-1])) {
    buffer[*size] = ' ';
    (*size)++;
  }

  buffer[*size] = c;
  (*size)++;
}

void
clear_buffer (char *buffer)
{
  memset(&buffer[0], 0, sizeof(buffer));
}

// Tested and is working properly
// tests if b is greater than a, ignore the fact that they're backwards
bool
is_greater_precedence (command_type b, command_type a)
{
  //determine which has greater precedence, true if a >
  if (a == SEQUENCE_COMMAND)
  {
    if (b == SEQUENCE_COMMAND)
      return false;
    else
      return true;
  }
  if (a == AND_COMMAND || a == OR_COMMAND)
  {
    if (b == SEQUENCE_COMMAND)
      return false;
    else if (b == AND_COMMAND || b == OR_COMMAND)
      return false;
    else
      return true;
  }
  if (a == PIPE_COMMAND)
    return false;
  return false;
}

// Tested this function and working properly
char**
tokenize_expression (char* buffer, int *size)
{
  int len = strlen(buffer);
  char** tokens = checked_malloc(len * sizeof(char*));
  char arr[DEFAULT_BUFFER_SIZE];
  strcpy(arr, buffer);
  char* temp = strtok(arr, " ");
  int i = 0;
  //int j = 1;
  while(temp)
  {
    tokens[i] = checked_malloc(DEFAULT_BUFFER_SIZE * sizeof(char));
    strcpy(tokens[i], temp);
    i++;
    if (i == len) {
      len *= 2;
      tokens = checked_realloc(tokens, len * sizeof(char*));
    }
    //j++;
    //tokens[] = realloc(tokens, j*sizeof(char*) * DEFAULT_BUFFER_SIZE);
    temp = strtok(NULL, " ");
  }
  *size = i;
  return tokens;
}

void
handle_operator (command_type op, stack *cmd_stack, stack *op_stack)
{
  if (op_stack->size == 0 || is_greater_precedence(op, peek(op_stack)->type)) {
    command_t new_op = new_command();
    new_op->type = op;
    push(op_stack, new_op);
  } else {
    command_type cur = op;
    while(cur != SUBSHELL_COMMAND && op_stack->size > 0 && !is_greater_precedence(op, peek(op_stack)->type)) {
      cur = pop(op_stack)->type;
      command_t cmd_a = pop(cmd_stack);
      command_t cmd_b = pop(cmd_stack);
      if (cmd_a != NULL && cmd_b != NULL) {
        command_t combined = new_command();
        combined->type = cur;
        combined->u.command[0] = cmd_b;
        combined->u.command[1] = cmd_a;
        push(cmd_stack, combined);
      } else {
        // throw error, why the fuck don't you have two commands
      }
    }
    command_t new_op = new_command();
    new_op->type = op;
    push(op_stack, new_op);
  }
}

void
handle_command (char **words, stack *cmd_stack, int num_words)
{
  command_t new_comm = new_command();
  char **commands = checked_malloc(sizeof(char*) * num_words + 1);
  int i;
  for (i = 0; i < num_words; i++) {
    commands[i] = checked_malloc(DEFAULT_BUFFER_SIZE * sizeof(char));
    strcpy(commands[i], words[i]);
    //fprintf(stderr, "%s\n", commands[i]);
    free(words[i]);
  }
  commands[i] = 0;
  new_comm->u.word = commands;
  new_comm->type = SIMPLE_COMMAND;
  push(cmd_stack, new_comm);
}

// Tested is working properly
bool
is_simple_command (char* token)
{
  unsigned int i;
  for (i = 0; i < strlen(token); i++) {
    //if (!isalnum(token[i])) return false;
    char c = token[i];
    if (c == '&' || c == '|') return false;
  }
  return true;
}

// builds the tree via the two stacks method
command_node*
process_expression (char *buffer)
{
  stack *cmd_stack = init_stack(DEFAULT_STACK_ITEMS);
  stack *op_stack = init_stack(DEFAULT_STACK_ITEMS);

  char **words = checked_malloc(sizeof(char*) * DEFAULT_WORDS);

  int size = 0;
  char** tokens = tokenize_expression(buffer, &size);

  command_node *cnode = checked_malloc(sizeof(command_node));
  cnode->command = checked_malloc(sizeof(struct command));

  int i;
  int word_number = 0;

  for (i = 0; i < size; i++) {
    char* token = tokens[i];

    // if the token is part of a simple command, shove it into a buffer
    if (is_simple_command(token)) {
      int buffer_max = DEFAULT_BUFFER_SIZE;
      int buffer_size = 0;
      //char *expr_buffer = malloc(sizeof(char) * buffer_max);

      /*unsigned int j;
      for (j = 0; j < strlen(token); j++) {
        buffer_append(token[j], expr_buffer, &buffer_size, &buffer_max);
      }*/
      words[word_number] = checked_malloc(sizeof(char) * buffer_max);
      words[word_number] = token;
      word_number++;
      //free(expr_buffer);
    } else if (token[0] == token[1]) { // now let's check if it's an operator!
      if (token[0] == '&') {
        handle_command(words, cmd_stack, word_number);
        word_number = 0;
        handle_operator(AND_COMMAND, cmd_stack, op_stack);
      } else if (token[0] == '|') {
        handle_command(words, cmd_stack, word_number);
        word_number = 0;
        handle_operator(OR_COMMAND, cmd_stack, op_stack);
      }
    } else if (strlen(token) == 1 && token[0] == '|') {
      handle_command(words, cmd_stack, word_number);
      word_number = 0;
      handle_operator(PIPE_COMMAND, cmd_stack, op_stack);
    }
  }
  if (word_number > 0)
    handle_command(words, cmd_stack, word_number);

  command_type cur;

  while (op_stack->size > 0 && cmd_stack->size > 1) {
    cur = pop(op_stack)->type;
    command_t cmd_a = pop(cmd_stack);
    command_t cmd_b = pop(cmd_stack);
    if (cmd_a != NULL && cmd_b != NULL) {
      command_t combined = new_command();
      combined->type = cur;
      combined->u.command[0] = cmd_b;
      combined->u.command[1] = cmd_a;
      push(cmd_stack, combined);
    }
  }

  // we should be guaranteed that the last thing on the command stack
  // is the root node of the tree...hopefully
  cnode->command = pop(cmd_stack);

  free(cmd_stack);
  free(op_stack);

  return cnode;
}

// adds a new node to the stream
void
append_node (command_node *node, command_stream_t stream)
{
  if (stream->head == NULL) {
    stream->head = node;
    stream->head->next = NULL;
    stream->iterator = NULL;
  } else if (stream->head->next == NULL){
    stream->head->next = node;
    stream->iterator = node;
    stream->iterator->next = NULL;
  } else {
    stream->iterator->next = node;
    stream->iterator = node;
  }
  stream->index++;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  command_stream_t stream = checked_malloc(sizeof(command_stream));
  stream->index = 0;
  stream->size = 0;
  stream->head = NULL;
  stream->iterator = NULL;

  int lines_read = 0;
  char *expression_buffer = checked_malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
  int buffer_size = 0;
  int buffer_max = DEFAULT_BUFFER_SIZE;

  char c;
  char last_char;

  while ((c = get_next_byte(get_next_byte_argument)) != EOF) {
    if (c == '\n') {
      lines_read++;
      if (last_char == '\n') {
        command_node *new_node = process_expression(expression_buffer);
        append_node(new_node, stream);
        clear_buffer(expression_buffer);
      } else {
        buffer_append(SEQUENCE_CHAR, expression_buffer, &buffer_size, &buffer_max);
      }
    /*} else if (c == ';' || c == '|' || c == '&' || isalnum(c) || c == ' ' || c == '/'
        || c == '>' || c == '<' || c == '!') {
      // replace this with a validation function eventually
      buffer_append(c, expression_buffer, &buffer_size, &buffer_max);*/
    } else {
      // throw error because invalid character
      buffer_append(c, expression_buffer, &buffer_size, &buffer_max);
    }
    last_char = c;
  }

  // now let's clean up the rest of it
  command_node *new_node = process_expression(expression_buffer);
  append_node(new_node, stream);
  clear_buffer(expression_buffer);

  free(expression_buffer);

  stream->iterator = stream->head;
  return stream;
}

command_t
read_command_stream (command_stream_t s)
{
  /*if (s->index == s->size) {
    s->index = 0;
    return NULL;
  }s

  s->index++;
  s->iterator = s->iterator->next;
  return s->iterator->command;*/
  if (s->iterator != NULL)
  {
    command_t result = s->iterator->command;
    s->iterator = s->iterator->next;
    return result;
  }
  else
    return NULL;
}
