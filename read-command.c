// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <stdio.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_STACK_ITEMS 10
#define SEQUENCE_CHAR ';'
#define PIPE_CHAR '|'
#define OR_SEQ "||"
#define AND_SEQ "&&"

typedef struct command_node command_node;
typedef struct command_stream command_stream;
typedef struct stack stack;
typedef enum { false, true } bool;

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
initialize_stack (int max)
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
  if (a == ';' || a == '\n')
  {
    if (b == ';' || b == '\n')
      return false;
    else
      return true;
  }
  if (a == '&&' || a == '||')
  {
    if (b == ';' || b == '\n')
      return false;
    else if (b == '&&' || b == '||')
      return false;
    else
      return true;
  }
  if (a == '|')
    return false;
}

char*
tokenize_simple_command (char* buffer)
{
  char* temp;
  char* tokens = checked_malloc(sizeof(char*));
  temp = strtok(buffer, ' ');
  int i = 0;
  int j = 1;
  while(temp != NULL)
  {
    tokens[i] = temp;
    i++;
    j++;
    tokens = checked_realloc(tokens, j*sizeof(char*));
    strtok(NULL, ' ');
  }
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
        command_t combined = NULL;
        combined->u.command[0] = cmd_a;
        combined->u.command[1] = cmd_b;
        cmd_stack->push(combined);
      }
    }
  }
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
  bool in_simple_command = false;
  char *simple_command_buffer = malloc(sizeof(char));
  stack *cmd_stack = initialize_stack(DEFAULT_STACK_ITEMS);
  stack *op_stack = initialize_stack(DEFAULT_STACK_ITEMS);

  char c;
  char last_char;

  //what the fuck are you doing
  //okay so first you have to read shit into a buffer
  //until you hit the DOUBLE NEWLINE or EOF
  //then parse that expression that you just made yo
  //basically this is shit and you should feel bad

  while (c != EOF) {
    c = get_next_byte(get_next_byte_argument);
    if (c == '\n') {
      lines_read++;
      if (last_char == '\n') {

      }
    } else if (c == ';' || c == '|' || c == '&') {
      if (in_simple_command == true) {
        in_simple_command = false;
        command_t new_cmd = NULL;
        tokenize_simple_command(simple_command_buffer, new_cmd);
        cmd_stack->push(new_cmd);
        memset(&simple_command_buffer[0], 0, sizeof(simple_command_buffer));
      }
      command_type ctype;
      switch(c)
      {
        case ';':
          command_type = SEQUENCE_COMMAND;
          break;
        case ''
      }
      handle_operator(c, cmd_stack, op_stack);
    }

    // when we finish an expression, load it into the node
    // then link it to a new node that we're now working on
    // if nothing exists,
    last_char = c;
  }

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
