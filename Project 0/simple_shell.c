#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

#define MAX_TOKEN_LENGTH 50
#define MAX_TOKEN_COUNT 100
#define MAX_LINE_LENGTH 512

//CHANGE TO 0 FOR RELEASE
#define DEBUG 1

// Simple implementation for Shell command
// Assume all arguments are seperated by space
// Erros are ignored when executing fgets(), fork(), and waitpid(). 

/**
 * Sample session
 *  shell: echo hello
 *   hello
 *   shell: ls
 *   Makefile  simple  simple_shell.c
 *   shell: exit
**/

//Simple Debugging Function to track errors
void error(int errno){
  if(DEBUG){
    printf("Error detected: ");
    switch(errno){
      case 1: printf("greater than three pipes requested\n"); break;
      case 2: printf("pipe creation failed\n"); break;
      default: printf("unspecified error\n");
    }
  }

  exit(errno);
}

char* specialToken[3] = { "<" , ">" , "|" };

void runcommand(char* command, char** args, int count) {

  // Notes from the TA
  // before fork separate via some kind of tokenizing on <, >, and |
  // take what is on the right side of delineator's output into what is on the left

  // Check the number of tokens given in the command line
  printf("Size %d \n", count);
  printf("Args are: \n ");

  //Count the number of pipes
  int pipeCount = 0;
  int pipes[3][2];
  
  for (int i = 0; i<count; i++){
    printf("           %s \n" , args[i]);
    
    if(strcmp(args[i],specialToken[0])==0) {
      // handle "<" function process
      printf("here is a <! \n");
    }
    
    if(strcmp(args[i],specialToken[1])==0) {
      // handle ">" function process
      printf("here is a >! \n");
    }
    
    if(strcmp(args[i],specialToken[2])==0) {
      // handle "|" function process"
      
      //Count the number of pipes needed
      //Piazza says max 3
      pipeCount++;

      printf("here is a |! \n");
    }
  }

  //Ensure Proper Amount of Pipes
  if(pipeCount > 3) error(1);

  //Initialize The Total Number of Pipes
  for(int i=0; i<pipeCount; i++){
    if(pipe(pipes[i]) < 0) error(2);
  }
  

  pid_t pid = fork();
  if(pid) { // parent
      waitpid(pid, NULL, 0);
  } else { // child
      execvp(command, args);
  }
}

//Based on Nik's implementation handles ctrl+z
void ctrlz(int sig){
    static int count = 0;
    count++;
    if (count == 1)
        signal(SIGTSTP, SIG_DFL);
}


int main(){
    signal(SIGTSTP, ctrlz);
    char line[MAX_LINE_LENGTH];
    //printf("shell: "); 
    while(fgets(line, MAX_LINE_LENGTH, stdin)) {
      // Build the command and arguments, using execv conventions.
      line[strlen(line)-1] = '\0'; // get rid of the new line
      char* command = NULL;
      char* arguments[MAX_TOKEN_COUNT];
      int argument_count = 0;

      char* token = strtok(line, " ");
      while(token) {
          if(!command) command = token;
          arguments[argument_count] = token;
          argument_count++;
          token = strtok(NULL, " ");
      }
      arguments[argument_count] = NULL;
  if(argument_count>0){
    if (strcmp(arguments[0], "exit") == 0)
                exit(0);
        runcommand(command, arguments, argument_count);
  }
        //printf("shell: "); 
    }
    return 0;
}