// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <stdio.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define DEFAULT_STACK_ITEMS 10
#define DEFAULT_BUFFER_SIZE 500
#define SEQUENCE_CHAR ';'
#define PIPE_CHAR '|'
#define OR_SEQ "||"
#define AND_SEQ "&&"

typedef struct command_node command_node;
typedef struct command_stream command_stream;
typedef struct stack stack;

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

void
push (stack *self, command_t command)
{
  if (self->size == self->max_size) {
    self->max_size *= 2;
    command_t *new = malloc (self->max_size * sizeof (command_t));
    self->commands = new;
  }

  self->commands[self->size] = command;
  self->size++;
}

command_t
pop (stack *self)
{
  if (self->size > 0)
    return self->commands[self->size--];
  else
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
  stack *new = malloc (sizeof (stack));
  new->size = 0;
  new->max_size = max;
  new->commands = malloc (max * sizeof (command_t));
}

bool
is_greater_precedence (command_type a, command_type b)
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
}

char**
tokenize_expression (char* buffer, int *size)
{
  char* temp;
  char** tokens = malloc(sizeof(char*));
  temp = strtok(buffer, ' ');
  int i = 0;
  int j = 1;
  while(temp != NULL)
  {
    tokens[i] = temp;
    i++;
    j++;
    tokens = realloc(tokens, j*sizeof(char*));
    strtok(NULL, ' ');
  }
  *size = i;
  return tokens;
}

void
handle_operator (command_type op, stack *cmd_stack, stack *op_stack)
{
  if (is_greater_precedence(op->type, op_stack->peek()->type)) {
    op_stack->push(op);
  } else if (op_stack->size == 0) {
    op_stack->push(op);
  } else {
    while(op->type != SUBSHELL_COMMAND && !is_greater_precedence(op->type, op_stack->pop()->type)) {
      command_t cmd_a = cmd_stack->pop();
      command_t cmd_b = cmd_stack->pop();
      if (cmd_a != NULL && cmd_b != NULL) {
        command_t combined = malloc(sizeof(command_t));
        combined->u.command[0] = cmd_a;
        combined->u.command[1] = cmd_b;
        cmd_stack->push(combined);
      }
    }
  }
}

// NOT YET IMPLEMENTED
// builds the tree via the two stacks method
command_node
process_expression (char *buffer)
{
  stack *cmd_stack = init_stack(DEFAULT_STACK_ITEMS);
  stack *op_stack = init_stack(DEFAULT_STACK_ITEMS);

  int size = 0;
  char **tokens = tokenize_expression(buffer, &size);

  command_node cnode;

  int i;
  for (i = 0; i < size; i++) {
    char* token = tokens[i];
    int tsize = strlen(token);
    int j;
    for (j = 0; j < tsize; j++)
    {
      char c = token[i];
      switch(c)
      {
        case ';':
        case '\n':
        {
          handle_operator (SEQUENCE_COMMAND, cmd_stack, op_stack);
          break;
        }
        case '|':
        {
          if (j == tsize-1 || token[j+1] != '|')
          {
            handle_operator (PIPE_COMMAND, cmd_stack, op_stack);
          }
          else
          {
            handle_operator (OR_COMMAND, cmd_stack, op_stack);
          }
          break;
        }
        case '&':
        {
          if (j == tsize-1 || token[j+1] != '&')
          {
            // Handle errors
          }
          else
          {
            handle_operator (AND_COMMAND, cmd_stack, op_stack);
          }
          break;
        }
        default:
          break;
      }
    }
  }
  int k;
  for (k = size; k >= 0; k--) {
    char* token = tokens[k];
    int tsize = strlen(token);
    int p;
    for (p = 0; p < tsize; p++)
    {
      char c = token[p];
      if (c == ';' || c == '\n')
      {
        cnode.command = SEQUENCE_COMMAND;
        break;
      }
      if (c =='|')
      {
        if (p == tsize-1 || token[p+1] != '|')
        {
          cnode.command = PIPE_COMMAND;
        }
        else
        {
          cnode.command = OR_COMMAND;
        }
          break;
      }
      if (c == '&')
      {
        if (p == tsize-1 || token[p+1] != '&')
        {
          // Handle errors
        }
        else
        {
          cnode.command = AND_COMMAND;
        }
        break;
      }
    }
  }
  return cnode;
}

// adds a new node to the stream
void
append_node (command_node node, command_stream_t stream)
{
  if (stream->head == NULL) {
    *(stream->head) = node;
    stream->head->next = NULL;
    stream->iterator = NULL;
  } else if (stream->head->next == NULL){
    *(stream->head)->next = node;
    *(stream->iterator) = node;
    stream->iterator->next = NULL;
  } else {
    *(stream->iterator)->next = node;
    *(stream->iterator) = node;
  }
  stream->index++;
}

// safe append for the buffer, reallocs as necessary
// adds a space before each operator for 420 offdarailz tokenization
void
buffer_append (char c, char *buffer, int *size, int *max)
{
  (*size)++;
  if (*size >= *max) {
    *max *= 2;
    char *new_buf = malloc(sizeof(char) * (*max));
    buffer = new_buf;
  }

  if (is_operator(c) && !is_operator(buffer[(*size)-1])) {
    buffer[*size] = ' ';
    (*size)++;
  }

  buffer[*size] = c;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  command_stream_t stream = malloc(sizeof(command_stream));
  stream->index = 0;
  stream->size = 0;
  stream->head = NULL;
  stream->iterator = NULL;

  int lines_read = 0;
  char *expression_buffer = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
  int buffer_size = 0;
  int buffer_max = DEFAULT_BUFFER_SIZE;

  char c;
  char last_char;

  while (c = get_next_byte(get_next_byte_argument) != EOF) {
    if (c == '\n') {
      lines_read++;
      if (last_char == '\n') {
        command_node new_node = process_expression(expression_buffer);
        append_node(new_node, stream);
        memset(&expression_buffer[0], 0, sizeof(expression_buffer));
      } else {
        buffer_append(SEQUENCE_CHAR, expression_buffer, &buffer_size, &buffer_max);
      }
    } else if (c == ';' || c == '|' || c == '&' || isalnum(c)) {
      // replace this with a validation function eventually
      buffer_append(c, expression_buffer, &buffer_size, &buffer_max);
    } else {
      // throw error because invalid character
    }
    last_char = c;
  }

  command_node new_node; process_expression(expression_buffer);
  append_node(new_node, stream);
  memset(&expression_buffer[0], 0, sizeof(expression_buffer));

  stream->iterator = stream->head;
  return stream;
}

command_t
read_command_stream (command_stream_t s)
{
  if (s->index == s->size) {
    s->index = 0;
    return NULL;
  }

  s->index++;
  s->iterator = s->iterator->next;
  return s->iterator->command;
}
