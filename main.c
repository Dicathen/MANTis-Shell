// CSCE 3600 - major 2 - group 6
// main.c
// author:  Nathan Jodoin
// date:    08APR23
// desc:    main function definition and basic shell logic

#include "main.h"
#include "path.h"
#include "parser.h"
#include "handle_exit.h"
#include <ctype.h>
#include "handle_myhistory.h"

void print_help() {
    printf( "\n"
      "MANTis - an interactive shell by Mason, Alex, Nathan, and Tobi\n"
      "Usage: newshell [batchfile]\n"
    );
}

// mallocs an uninitialized char* and sets it to a custom prompt
char* set_prompt(char * prompt) {
  prompt = (char*)malloc(sizeof(char)*11);
  size_t prompt_len = sizeof(char)*11;

  if ( prompt == NULL ) {
    perror("ERROR: Unable to malloc prompt vars in set_prompt. " );
    return NULL;
  }

  printf("Would you like to set a custom shell prompt?\n[Default -->:] Y/n: ");
  if ( getline(&prompt, &prompt_len, stdin) == -1 ) {
      perror("User input too long or error reading from stdin ");
      return NULL;
  }
  for ( int i=0; i < strlen(prompt); i++ )
    prompt[i] = tolower(prompt[i]);

  if ( strcmp(prompt, "y\n") == 0 ) {
    printf("Please enter your custom shell prompt\n(<= 10 chars): ");
    if ( getline(&prompt, &prompt_len, stdin) == -1 ) {
        perror("User input too long or error reading from stdin ");
        return NULL;
    }

    else if ( strcmp(prompt, "\n") == 0 || strlen(prompt) > 11) {
      printf("Prompt too long or empty. Setting default prompt.\n");
      strcpy(prompt, "-->");
    }

  }
  else {
    strcpy(prompt, "-->");
  }
  prompt = strtok(prompt, "\n");
  return prompt;
}

void prompt(const char* prompt) {
  printf("\n%s: ", prompt);
}

int main( int cargs, char** argv ) {

  int num_args;
  char* cust_prompt;
  bool exit_flag = false;
  size_t user_in_len = MAX_ARG_LEN;
  char* shell_dir = (char*) calloc(PATH_MAX, sizeof(char));

  // PATH_MAX is a system defined macro for the max filepath length.
  char* user_in = (char*) calloc(MAX_ARG_LEN, sizeof(char));
  char** args_buff = (char**) calloc(MAX_ARG_LEN, sizeof(char*));

  if (setup_exit() == -1) {
    perror("ERROR: unable to setup signal handler. ");
    return 1;
  }

  printf(
      // We should see if we can come up with a better name...
      "\nWelcome to MANTis, A Basic Interactive Shell Built for CSCE3600\n\n"
      );

  if ( shell_dir == NULL || args_buff == NULL || user_in == NULL ) {
    errno = ENOMEM;
    perror("ERROR: unable to malloc during shell init.");
    return 1;
  }

  //get current working directory so we always know where the PATH is
  if ( getcwd(shell_dir, PATH_MAX) == NULL ) {
    perror("ERROR: getcwd failed ");
    return 1;
  }

  // usage/help statement
  if ( cargs == 2 && strcmp(argv[1], "-help") == 0 ) {
    print_help();
    return 0;
  }

  // interactive mode
  else if ( cargs == 1 ) {

    cust_prompt = set_prompt(cust_prompt);
    // Interactive user input loop:
    printf(
        "\nBegin interactive mode...\n"
        "Enter \"exit\" to exit the shell. \n");
    prompt(cust_prompt);
    if ( getline(&user_in, &user_in_len, stdin) == -1 ) {
      perror("User input too long or error reading from stdin ");
      return 1;
    }

    while ( true ) {

      if (args_buff == NULL ){
        perror("ERROR: Unable to malloc args_buff in get_args ");
        return 1;
      }

      num_args = get_args(args_buff, user_in);
      parse_args(args_buff, num_args, &exit_flag);

      for ( int i = 0; i < num_args; i++) {
        free(args_buff[i]);
      }

      if (exit_flag) {
        break;
      }

      add_to_history(user_in);
      prompt(cust_prompt);
      if ( getline(&user_in, &user_in_len, stdin) == -1 ) {
        perror("User input too long or error reading from stdin ");
        return 1;
      }
    }
  }

  // batch mode
  else if ( cargs == 2 ) {
    char* line = (char*)malloc(MAX_ARG_LEN*sizeof(char));
    printf("Batch mode.\n\n");

    if (args_buff == NULL ){
      perror("ERROR: Unable to malloc args_buff in get_args ");
      return 1;
    }

    num_args = get_args_from_batch(args_buff, argv[1]);

    for( int i = 0; i < num_args; i++) {
      strcpy(line, args_buff[i]);
      parse_args(args_buff, num_args, &exit_flag);
      if (exit_flag)
        break;
    }
    for (int i = 0; i < num_args; i++) {
      free(args_buff[i]);
    }
    if (line != NULL)
      free(line);
  }
  else if ( cargs > 2 || cargs < 1 ) {
    errno = EINVAL;
    perror("Incorrect number of arguments. ");
    return 1;
  }

  free(cust_prompt);
  free(shell_dir);
  free(args_buff);
  free(user_in);
  return 0;
}
