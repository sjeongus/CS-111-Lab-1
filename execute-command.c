// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#define GRAPH_SIZE 30
#define DEFAULT_WORDS 5

struct list_node {
  char* word;
  struct list_node *next;
};

struct list {
  list_node *head;
  int size;
};

struct graph_node {
  command_t command;
  pid_t pid;
  struct graph_node **before;
  int words;
  int max_words;
};

struct queue_node {
  graph_node *node;
  list* read_list;
  list* write_list;
};

struct dependency_graph {
  queue_node* no_dependencies[GRAPH_SIZE];
  queue_node* dependencies[GRAPH_SIZE];
  int num_nodepen;
  int num_depen;
};

void
list_insert (list *self, char* word)
{
  if (word == NULL) return;
  list_node *node = malloc(sizeof(list_node));
  node->word = word;
  if (self->head == NULL) {
    node->next = NULL;
    self->head = node;
  } else {
    list_node *cur = self->head;
    while (strcmp(cur->word, word) >= 0) {
      if (cur->next != NULL)
        cur = cur->next;
      else break;
    }
    list_node *next = NULL;
    if (cur->next != NULL)
      next = cur->next->next;
    cur->next = node;
    node->next = next;
  }
  self->size++;
}

void
list_remove (list *self)
{
  if (self->head == NULL)
    return;
  else if (self->head->next == NULL) {
    list_node *top = self->head;
    self->head = NULL;
    free(top);
  } else {
    list_node *top = self->head;
    self->head = self->head->next;
    free(top);
  }
}

list*
init_list ()
{
  list *new = checked_malloc(sizeof(list));
  new->size = 0;
  new->head = NULL;
  return new;
}

int
command_status (command_t c)
{
  return c->status;
}

void executingSimple(command_t c);
void executingSubshell(command_t c);
void executingAnd(command_t c);
void executingOr(command_t c);
void executingSequence(command_t c);
void executingPipe(command_t c);

void execute_switch(command_t c)
{
  switch(c->type)
  {
    case SIMPLE_COMMAND:
      executingSimple(c);
      break;
    case SUBSHELL_COMMAND:
      executingSubshell(c);
      break;
     case AND_COMMAND:
      executingAnd(c);
      break;
    case OR_COMMAND:
      executingOr(c);
      break;
    case SEQUENCE_COMMAND:
      executingSequence(c);
      break;
    case PIPE_COMMAND:
      executingPipe(c);
      break;
    default:
      error(1, 0, "Not a valid command");
  }
}

void executingSimple(command_t c)
{
  int status;
  pid_t pid = fork();
  if (pid > 0)
  {
    // Wait for child, then store status
    waitpid(pid, &status, 0);
    c->status = status;
  }
  else if (pid == 0)
  {
    if (c->input != NULL)
    {
      int fd_in = open(c->input, O_RDWR);
      if (fd_in < 0)
        error(1, 0, "Couldn't read input file: %s", c->input);
      if (dup2(fd_in, 0) < 0)
        error(1, 0, "Error with dup2 for input file: %s", c->input);
      if (close(fd_in) < 0)
        error(1, 0, "Couldn't close input file: %s", c->input);
    }
    if (c->output != NULL)
    {
      int fd_out = open(c->output, O_CREAT | O_WRONLY | O_TRUNC,
        S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
      if (fd_out < 0)
        error(1, 0, "Couldn't read output file: %s", c->output);
      if (dup2(fd_out, 1) < 0)
        error(1, 0, "Error with dup2 for output file: %s", c->output);
      if (close(fd_out) < 0)
        error(1, 0, "Couldn't close output file: %s", c->output);
    }
    if (c->u.word[0][0] == ':')
      _exit(0);
    // Execute simple command
    execvp(c->u.word[0], c->u.word);
    error(1, 0, "Invalid simple command");
  }
  else
    error(1, 0, "Could not fork");
}

void executingSubshell(command_t c)
{
  if (c->type == SIMPLE_COMMAND)
    executingSimple(c);
  else if (c->type == SUBSHELL_COMMAND)
  {
    if (c->input != NULL)
    {
      int fd_in = open(c->input, O_RDWR);
      if (fd_in < 0)
        error(1, 0, "Couldn't read input file: %s", c->input);
      if (dup2(fd_in, 0) < 0)
        error(1, 0, "Error with dup2 for input file: %s", c->input);
      if (close(fd_in) < 0)
        error(1, 0, "Couldn't close input file: %s", c->input);
    }
    if (c->output != NULL)
    {
      int fd_out = open(c->output, O_CREAT | O_WRONLY | O_TRUNC,
        S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
      if (fd_out < 0)
        error(1, 0, "Couldn't read output file: %s", c->output);
      if (dup2(fd_out, 1) < 0)
        error(1, 0, "Error with dup2 for output file: %s", c->output);
      if (close(fd_out) < 0)
        error(1, 0, "Couldn't close output file: %s", c->output);
    }

    execute_switch(c->u.subshell_command);
    c->status = 0;
  }
  else
    error (1, 0, "Error with subshells.");
}

void executingAnd(command_t c)
{
  execute_switch(c->u.command[0]);
  if (c->u.command[0]->status == 0)
  {
    execute_switch(c->u.command[1]);
    c->status = c->u.command[1]->status;
  }
  else
    c->status = c->u.command[0]->status;
}

void executingOr(command_t c)
{
  execute_switch(c->u.command[0]);
  if (c->u.command[0]->status != 0)
  {
    execute_switch(c->u.command[1]);
    c->status = c->u.command[1]->status;
  }
  else
    c->status = c->u.command[0]->status;
}

void executingSequence(command_t c)
{
  int status;
  pid_t pid = fork();
  if (pid > 0)
  {
    waitpid(pid, &status, 0);
    c->status = status;
  }
  else if (pid == 0)
  {
    pid = fork();
    if (pid > 0)
    {
      waitpid(pid, &status, 0);
      execute_switch(c->u.command[1]);
      _exit(c->u.command[1]->status);
    }
    else if (pid == 0)
    {
      execute_switch(c->u.command[0]);
      _exit(c->u.command[0]->status);
    }
    else error(1, 0, "fork was unsuccessful");
  }
  else
    error(1, 0, "fork was unsuccessful");
}

void executingPipe(command_t c)
{
  pid_t returnedPid;
  pid_t firstPid;
  pid_t secondPid;
  int buffer[2];
  int eStatus;

  if ( pipe(buffer) < 0 )
  {
    error (1, errno, "pipe was not created");
  }

  firstPid = fork();
  if (firstPid < 0)
  {
    error(1, errno, "fork was unsuccessful");
  }
  else if (firstPid == 0) //child executes command on the right of the pipe
  {
    close(buffer[1]); //close unused write end
           //redirect standard input to the read end of the pipe
           //so that input of the command (on the right of the pipe)
           //comes from the pipe
    if ( dup2(buffer[0], 0) < 0 )
    {
      error(1, errno, "error with dup2");
    }
    execute_switch(c->u.command[1]);
    _exit(c->u.command[1]->status);
  }
  else
  {
  // Parent process
    secondPid = fork(); //fork another child process
                       //have that child process executes command on the left of the pipe
    if (secondPid < 0)
    {
      error(1, 0, "fork was unsuccessful");
    }
    else if (secondPid == 0)
    {
      close(buffer[0]); //close unused read end
      if(dup2(buffer[1], 1) < 0) //redirect standard output to write end of the pipe
      {
        error (1, errno, "error with dup2");
      }
      execute_switch(c->u.command[0]);
      _exit(c->u.command[0]->status);
    }
    else
    {
      // Finishing processes
      returnedPid = waitpid(-1, &eStatus, 0); //this is equivalent to wait(&eStatus);
                      //we now have 2 children. This waitpid will suspend the execution of
                      //the calling process until one of its children terminates
                      //(the other may not terminate yet)
      //Close pipe
      close(buffer[0]);
      close(buffer[1]);
      if (secondPid == returnedPid )
      {
        //wait for the remaining child process to terminate
        waitpid(firstPid, &eStatus, 0);
        c->status = WEXITSTATUS(eStatus);
        return;
      }
      if (firstPid == returnedPid)
      {
        //wait for the remaining child process to terminate
        waitpid(secondPid, &eStatus, 0);
        c->status = WEXITSTATUS(eStatus);
        return;
      }
    }
  }
}

void
execute_command (command_t c, bool time_travel)
{
  //error (1, 0, "command execution not yet implemented");
  execute_switch(c);
}

// this should work
void
process_command(command_t cmd, queue_node *node)
{
  if (cmd->type == SIMPLE_COMMAND) {
    int i = 0;
    while (cmd->u.word[i] != NULL) {
      list_insert(node->read_list, cmd->u.word[i]);
      i++;
    }
    if (cmd->input != NULL)
      list_insert(node->read_list, cmd->input);
    if (cmd->output != NULL)
      list_insert(node->write_list, cmd->output);
  } else if (cmd->type == SUBSHELL_COMMAND) {
    if (cmd->input != NULL)
      list_insert(node->read_list, cmd->input);
    if (cmd->output != NULL)
      list_insert(node->write_list, cmd->output);
    process_command(cmd->u.subshell_command, node);
  } else {
    process_command(cmd->u.command[0], node);
    process_command(cmd->u.command[1], node);
  }
}

// returns true if any element of a is in b
bool
contains (list *a, list *b)
{
  list_node *a_i = a->head;
  int i = 0;
  while (i < a->size) {
    if (a_i == NULL) break;
    list_node *b_i = b->head;
    int j = 0;
    while (j < b->size) {
      if (strcmp(a_i->word, b_i->word) == 0)
        return true;
      b_i = b_i->next;
      j++;
    }
    a_i = a_i->next;
    i++;
  }
  return false;
}

void
build_dependencies(queue_node *qnode, dependency_graph *graph)
{
  int i;
  for (i = 0; i < GRAPH_SIZE; i++) {
    if (i > graph->num_nodepen && i > graph->num_depen) break;

    queue_node *ndp_node = graph->no_dependencies[i];
    queue_node *dp_node = graph->dependencies[i];

    // check each read/write list of both no dependencies and dependencies
    // if any dependency is found, add that node into before
    if (i < graph->num_nodepen) {
      if (contains(qnode->read_list, ndp_node->write_list) ||
          contains(qnode->write_list, ndp_node->read_list) ||
          contains(qnode->write_list, ndp_node->write_list)) {
          qnode->node->before[qnode->node->words] = ndp_node->node;
          qnode->node->words = qnode->node->words + 1;
          if (qnode->node->words > qnode->node->max_words) {
            qnode->node->max_words *= 2;
            qnode->node->before = checked_realloc(qnode->node->before, qnode->node->max_words);
          }
      }
    }
    if (i < graph->num_depen) {
      if (contains(qnode->read_list, dp_node->write_list) ||
          contains(qnode->write_list, dp_node->read_list) ||
          contains(qnode->write_list, dp_node->write_list)) {
          qnode->node->before[qnode->node->words] = dp_node->node;
          qnode->node->words = qnode->node->words + 1;
          if (qnode->node->words > qnode->node->max_words) {
            qnode->node->max_words *= 2;
            qnode->node->before = checked_realloc(qnode->node->before, qnode->node->max_words);
          }
      }
    }
  }

  if (qnode->node->before == NULL) {
    graph->no_dependencies[graph->num_nodepen] = qnode;
    graph->num_nodepen = graph->num_nodepen + 1;
  } else {
    graph->dependencies[graph->num_depen] = qnode;
    graph->num_depen = graph->num_depen + 1;
  }
}

dependency_graph*
create_graph(command_stream_t stream)
{
  dependency_graph *graph = checked_malloc(sizeof(dependency_graph));
  graph->num_nodepen = 0;
  graph->num_depen = 0;
  command_t cur;
  while ((cur = read_command_stream(stream))) {
    queue_node *qnode = checked_malloc(sizeof(queue_node));
    qnode->node = checked_malloc(sizeof(graph_node));
    qnode->read_list = init_list();
    qnode->write_list = init_list();
    qnode->node->command = cur;
    qnode->node->pid = -1;
    int length = 0;
    int max_length = DEFAULT_WORDS;
    qnode->node->before = checked_malloc(sizeof(graph_node) * max_length);
    qnode->node->words = 0;
    qnode->node->max_words = max_length;
    process_command(cur, qnode);
    build_dependencies(qnode, graph);
  }
  return graph;
}

void
execute_no_dependencies(queue_node** ndp)
{
  int i;
  for (i = 0; i < GRAPH_SIZE; i++) {
    if (ndp[i] == NULL) break;
    pid_t pid = fork();
    if (pid == 0) {
      execute_command(ndp[i]->node->command, true);
      _exit(ndp[i]->node->command->status);
    }
    if (pid > 0) {
      ndp[i]->node->pid = pid;
    }
  }
}

void
execute_dependencies(queue_node** dp)
{
  int i;
  for (i = 0; i < GRAPH_SIZE; i++) {
    if (dp[i] == NULL || dp[i]->node == NULL) break;
    looplabel: ;
    int j;
    // polling
    for (j = 0; j < dp[i]->node->before[j] != NULL; j++) {
      if (dp[i]->node->before[j]->pid == -1)
        goto looplabel;
    }
    int status;
    for (j = 0; j < dp[i]->node->before[j] != NULL; j++) {
      waitpid(dp[i]->node->before[j]->pid, &status, 0);
    }
    pid_t pid = fork();
    if (pid < 0) {
      error(1, errno, "Unsuccessful fork.");
    }
    if (pid == 0) {
      execute_command(dp[i]->node->command, true);
      _exit(dp[i]->node->command->status);
    }
    if (pid > 0) {
      dp[i]->node->pid = pid;
    }
  }
}

void
execute_graph(dependency_graph *graph)
{
  execute_no_dependencies(graph->no_dependencies);
  execute_dependencies(graph->dependencies);
  int i, status;
  for (i = 0; i < graph->num_nodepen; i++)
    waitpid(graph->no_dependencies[i]->node->pid, &status, 0);
  for (i = 0; i < graph->num_depen; i++)
    waitpid(graph->dependencies[i]->node->pid, &status, 0);
}
