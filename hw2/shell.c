#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>


#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

void sig_int(int signo);
void sig_quit(int signo);

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);


/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_exit, "exit", "exit the command shell"},
  {cmd_pwd, "pwd", "show current path"},
  {cmd_cd, "cd", "change directory"},
  
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(struct tokens *tokens) {
  tokens_destroy(tokens);
  exit(0);
}

int cmd_pwd(unused struct tokens *tokens) {
  char cwd[PATH_MAX];
   if (getcwd(cwd, sizeof(cwd)) != NULL) {
       printf("%s\n", cwd);
   } else {
       perror("path error");
       return 0;
   }
   return 1;
}

int cmd_cd(struct tokens *tokens) {
  char * arg = tokens_get_token(tokens, 1);
  if (arg != NULL) {
    chdir(arg);
  } else {
    perror("no argument");
    return 0;
  }
  return 1;
}

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

/* Intialization procedures for this shell */
void init_shell() {
  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);
  

  if (shell_is_interactive) {
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

char* replace_char(char* str, char find, char replace){
  char *current_pos = strchr(str,find);
  while (current_pos){
    *current_pos = replace;
    current_pos = strchr(current_pos,find);
  }
  return str;
}
/* 
int isFileExists(const char *path)
{
    // Try to open file
    FILE *fptr = fopen(path, "r");

    // If file does not exists 
    if (fptr == NULL) {
        return 0;
    }
    // File exists hence close file and return true.
    fclose(fptr);

    return 1;
}
*/

int main(unused int argc, unused char *argv[]) {
  init_shell();

  static char line[4096];
  int line_num = 0;

  
  struct sigaction sigign; //ignore
  struct sigaction sigdfl; //default

  sigign.sa_handler = SIG_IGN;  //handler
  sigemptyset(&sigign.sa_mask); //make zero values in struct sigact
  sigign.sa_flags = 0;

  sigdfl.sa_handler = SIG_DFL;  //handler
  sigemptyset(&sigdfl.sa_mask); //make zero values in struct sigact
  sigdfl.sa_flags = 0;

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);
  
  if (sigaction(SIGINT, &sigign, 0) == -1) {
    printf ("signal(SIGINT) error");
          return -1;
  }  

  if (sigaction(SIGQUIT, &sigign, 0) == -1) {
    printf ("signal(SIGQUIT) error");
          return -1;
  }  

  if (sigaction(SIGTERM, &sigign, 0) == -1) {
    printf ("signal(SIGTERM) error");
          return -1;
  } 

  if (sigaction(SIGTSTP, &sigign, 0) == -1) {
    printf ("signal(SIGTSTP) error");
          return -1;
  }  

  if (sigaction(SIGCONT, &sigign, 0) == -1) {
    printf ("signal(SIGCONT) error");
          return -1;
  } 

  if (sigaction(SIGTTIN, &sigign, 0) == -1) {
    printf ("signal(SIGTTIN) error");
          return -1;
  } 

  if (sigaction(SIGTTOU, &sigign, 0) == -1) {
    printf ("signal(SIGTTOU) error");
          return -1;
  } 


  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
    struct tokens *tokens = tokenize(line);

    /* Find which built-in function to run. */
    int fundex = lookup(tokens_get_token(tokens, 0));
    FILE *fp = NULL;
    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else {
      /* REPLACE this to run commands as programs. */
      //fprintf(stdout, "This shell doesn't know how to run programs.\n");

      pid_t pid = fork();
      

      
      //printf("group_id::: '%d'\n", group_id);
      //printf("set_group_id::: '%d'\n", set_pgid);
      if (pid != 0) {
        //printf("parent_id::: '%d'\n", getpid());
        
        wait(0);
      }
      else {
        
        //setpgid(pid, pid);
        int child_groupID = getpgrp();
        tcsetpgrp(0, child_groupID);
        //printf("group_id::: '%d'\n", getpgrp());
  
        if (sigaction(SIGINT, &sigdfl, 0) == -1) {
          printf ("signal(SIGINT) error");
          return -1;
        }  

        if (sigaction(SIGQUIT, &sigdfl, 0) == -1) {
          printf ("signal(SIGQUIT) error");
          return -1;
        }  

        if (sigaction(SIGTERM, &sigdfl, 0) == -1) {
          printf ("signal(SIGTERM) error");
          return -1;
        }  

        if (sigaction(SIGTSTP, &sigdfl, 0) == -1) {
          printf ("signal(SIGTSTP) error");
          return -1;
        }  

        if (sigaction(SIGCONT, &sigdfl, 0) == -1) {
          printf ("signal(SIGCONT) error");
          return -1;
        } 

        if (sigaction(SIGTTIN, &sigdfl, 0) == -1) {
          printf ("signal(SIGTTIN) error");
          return -1;
        } 

        if (sigaction(SIGTTOU, &sigdfl, 0) == -1) {
          printf ("signal(SIGTTOU) error");
          return -1;
        }  

        char *args[2048]; //in stack
        size_t size = tokens_get_length(tokens);
        int i = 0;
        for (; i < size; i++) {
          char *arg = tokens_get_token(tokens, i);
          //printf("agr::: '%s'\n", arg);
          if (*arg == '>') {
             //printf("agr_print::: '%s'\n", arg);
             char *next_token = tokens_get_token(tokens, i + 1);
             if (next_token == NULL) {
               perror("no file name provided");
             } else {
              char *file_name = next_token;
              //printf("file_name::: '%s'\n", file_name);
              fp = freopen(file_name ,"w", stdout);
              break;
             }
          }
          else if (*arg == '<') {
            char *next_token = tokens_get_token(tokens, i + 1);
            if (next_token == NULL) {
               perror("no file name provided");
             } else {
               char *file_name = next_token;
              //printf("file_name in::: '%s'\n", file_name);
              fp = freopen(file_name ,"r", stdin);
              break;
             }
          }
          args[i] = arg;
        }
        args[i + 1] = NULL;
        
        char *first_arg = tokens_get_token(tokens, 0);
        //printf("first_agr::: '%s'\n", first_arg);

        char *path = getenv("PATH");
        
        char *ret = replace_char(path, ':', ' ');
        
        struct tokens *paths = tokenize(ret);
        size_t path_len = tokens_get_length(paths);

        for (int i = 0; i < path_len; i++) {
          char *arg = tokens_get_token(paths, i);
          char *add_slash = malloc(sizeof(char) * (strlen(arg) + 1));
          strcpy(add_slash, arg);
          strcat (add_slash, "/");

          char *res = strcat (add_slash, first_arg);
          
          if (access(res, X_OK) == 0) { // access([pathname], X_OK)--> if returns 0, ok else...
            first_arg = res;
            break;
            
            
          } 
        }
        
        
        execv(first_arg, args);
        
        fclose(fp);
        exit(0);

      }


    }

    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);

    /* Clean up memory */
    tokens_destroy(tokens);
  }

  return 0;
}
