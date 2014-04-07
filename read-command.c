// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <stdio.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_STACK_ITEMS 10
#define DEFAULT_BUFFER_SIZE 500
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
  //determine which has greater precedence, true if a > b
}

typedef enum
{
  NONE,
  WORD,
  SEMICOLON,
  NEWLINE,
  PIPE,
  AND,
  OR,
  BEGIN_SUBSHELL,
  END_SUBSHELL,
  INPUT,
  OUTPUT
} token_type;

typedef struct token token;
struct token
{
  char* input_str;
  token_type type;
};

typedef struct tokensarr tokensarr;
struct tokensarr
{
  token* aToken;
  int length;
};

token*
append_token(token* old_list, int old_length, token input)
{
  old_list = checked_realloc(old_list, ((old_length+1)*sizeof(token)));
  *(old_list + old_len) = input;
  return old_list;
}

void
tokenize_simple_command (char* buffer, command_t command)
{
  tokensarr* tarr = checked_malloc(sizeof(tokensarr));
  tarr->aToken = NULL;
  tarr->length = 0;

  token curr;
  curr.input_str = NULL;
  curr.type = NONE;

  int bufferlen = strlen(buffer);
  int i;
  for (i = 0; i < bufferlen-1; i++)
  {
    char c = buffer[i];
    switch (c)
    {
      case ';':
      case '\n':
      {
        if (curr.type != NONE)
        {
          tarr->aToken = append_token(tarr->aToken, tarr->length, curr);
          tarr->length++;
        }

        char* new_str = checked_malloc(2*sizeof(char));
        new_str[0] = c;
        new_str[1] = '\0';

        curr.token_str = new_str;
        if (c == ';')
          curr.type = SEMICOLON;
        else
          curr.type = NEWLINE;
        break;
      }
      case '|':
      {
        if (curr.type != NONE)
        {
          tarr->aToken = append_token(tarr->aToken, tarr->length, curr);
          tarr->length++;
        }
        if (i == bufferlen-1 || buff[i+1] != '|')
        {
          char* new_str = checked_malloc(2*sizeof(char));
          new_str[0] = c;
          new_str[1] = '\0';
          curr.input_str = new_str;
          curr.type = PIPE;
        }
        else
        {
          char* new_str = checked_malloc(3*sizeof(char));
          new_str[0] = c;
          new_str[1] = buffer[i+1];
          new_str[2] = '\0';
          curr.input_str = new_str;
          curr.type = OR;
          i++;
        }
        break;
      }
      case '&':
      {
        if (curr.type != NONE)
        {
          tarr->aToken = append_token(tarr->aToken, tarr->length, curr);
          tarr->length++;
        }
        if (i == bufferlen-1 || buffer[i+1] != '&')
        {
          //handle error
        }
        else
        {
          char* new_str = checked_malloc(3*sizeof(char));
          new_str[0] = c;
          new_str[1] = buffer[i+1];
          new_str[2] = '\0';
          curr.input_str = new_str;
          curr.type = AND;
          i++;
        }
        break;
      }
      case '(':
      {
        if (curr.type != NONE)
        {
          tarr->aToken = append_token(tarr->aToken, tarr->length, curr);
          tarr->length++;
        }
        char* new_str = checked_malloc(2*sizeof(char));
        new_str[0] = c;
        new_str[1] = '\0';
        curr.input_str = new_str;
        curr.type = BEGIN_SUBSHELL;
        break;
      }
      case ')':
      {
        if (curr.type != NONE)
        {
          tarr->aToken = append_token(tarr->aToken, tarr->length, curr);
          tarr->length++;
        }
        char* new_str = checked_malloc(2*sizeof(char));
        new_str[0] = c;
        new_str[1] = '\0';
        curr.input_str = new_str;
        curr.type = END_SUBSHELL;
        break;
      }
      case '<':
      {
        if (curr.type != NONE)
        {
          tarr->aToken = append_token(tarr->aToken, tarr->length, curr);
          tarr->length++;
        }
        char* new_str = checked_malloc(2*sizeof(char));
        new_str[0] = c;
        new_str[1] = '\0';
        curr.input_str = new_str;
        curr.type = INPUT;
        break;
      }
      case '>':
      {
        if (curr.type != NONE)
        {
          tarr->aToken = append_token(tarr->aToken, tarr->length, curr);
          tarr->length++;
        }
        char* new_str = checked_malloc(2*sizeof(char));
        new_str[0] = c;
        new_str[1] = '\0';
        curr.input_str = new_str;
        curr.type = OUTPUT;
        break;
      }
      case '\t':
      case ' ':
      {
        if (curr.type != WORD)
          break;
      }
      default:
      {
        if (curr.type != WORD)
        {
          tarr->aToken = append_token(tarr->aToken, tarr->length, curr);
          tarr->length++;
        }
        curr.type = WORD;
        curr.input_str = checekd_malloc(sizeof(char));
        *curr.input_str = '\0';
      }
      int length = strlen(curr.input_str) + 1;
      curr.input_str = checked_realloc(string, length);
      curr.input_str[length-1] = c;
      curr.input_str[length] = 0;
      break;
    }
  }
  if (curr.type != NONE)
  {
    tarr->aToken = append_token(tarr->aToken, tarr->length, curr);
    tarr->length++;
  }
  return tarr;
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
}

// adds a new node to the stream
void
append_node (command_node node, command_stream_t stream)
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

// safe append for the buffer, reallocs as necessary
// adds a space before each operator for 420 offdarailz tokenization
void
buffer_append (char c, char *buffer, int *size, int *max)
{
  size++;
  if (size >= max) {
    max *= 2;
    char *new_buf = malloc(sizeof(char) * max);
    buffer = new_buf;
  }

  if (is_operator(c) && !is_operator(buffer[size-1])) {
    buffer[size] = ' ';
    size++;
  }

  buffer[size] = c;
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

  // what the fuck are you doing
  // okay so first you have to read shit into a buffer
  // until you hit the DOUBLE NEWLINE or EOF
  // then parse that expression that you just made yo
  // basically this is shit and you should feel bad

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

  command_node new_node process_expression(expression_buffer);
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
