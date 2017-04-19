#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

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
      case 3: printf("Failed to dup STDIN\n"); break;
      case 4: printf("Failed to dup STDOUT\n"); break;
      default: printf("unspecified error\n");
    }
  }

  exit(errno);
}


char* specialToken[3] = { "<" , ">" , "|" };



void endCarrots(int lastCommand, char** args, int count){
	// Base Case: Number of Pipes = 0 
	int index = count - 2;
	while(index >= lastCommand){
	    // handle "<" function process
	    if(strcmp(args[index],specialToken[0])==0) {
	      //args[i] = args[i+1];
	      
	      
			int fd;
			fd = open(args[index+1], O_RDONLY);
			// Check if file exists
			if(fd<0){ 
		  		perror("No such file or directory\n");
			}
			args[index] = NULL;
			dup2(fd,0);
			close(fd);
	    }
	    // handle ">" function process
	    else if(strcmp(args[index],specialToken[1]) == 0){
	    	args[index] = NULL;
	    	pid_t fd;
	    	fd = open(args[index+1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
	    	
	    	dup2(fd,1);
	    	close(fd);
	    }
		index = index-2;
	}
	   
}


void runcommand(char* command, char** args, int count) {
	// Notes from the TA
  	// before fork separate via some kind of tokenizing on <, >, and |
 	// take what is on the right side of delineator's output into what is on the left

  	// Check the number of tokens given in the command line
  	//printf("Size %d \n", count);
  	//printf("Args are: \n ");

  	//Count the number of pipes
  	int pipeCount = 0;
  	int pipes[3][2];
  	char** commands[4];
  	//Number of commands followed by a pipe '|'
  	int pipeCommands = 0;
  	//Index of the last command
  	int lastCommand = 0; 

  
  	//Keep Track of number of arguments in the current command
  	int currentCount = 0;
  	for (int i = 0; i<count; i++){

   		//Handle "|" function process
  
    	if(strcmp(args[i],specialToken[2])==0) {
     		//Create New Pipe
      		if(pipe(pipes[pipeCount]) < 0) error(2);
      		pipeCount++;

      		//Create Partitioned Commands
      		commands[pipeCommands] = &args[i-currentCount];
      		args[i] = NULL; //Physical Partition
      		pipeCommands++;

      		//Guess the index of the last command
      		lastCommand = i+1;

      		//Reset Current Count
      		currentCount = 0;
      		continue;
    	}

    	//Increment count of items in command
    	currentCount++;
	}

	//Put last command in command
	commands[pipeCommands] = &args[lastCommand];

  	//Ensure Proper Amount of Pipes
  	if(pipeCount > 3) error(1);

  	//Execute Piped Commands
  	for(int i=0; i<pipeCount; i++){
    	pid_t pid = fork();
    	if(pid) { // parent
      		if(i > 0 && i < pipeCount){
        		close(pipes[i-1][0]);
        		close(pipes[i-1][1]);
      		}
      		waitpid(pid, NULL, 0);
    	} 
    	else { // child
    
			int index = (commands[i] - commands[0] )/ sizeof(char*);
			int size = (*commands[i+1] - *commands[i]);
			printf("Args[i] %p, args[i+2] %p\n", args[0], args[2]);
			printf("Commands i %p, commands i+1 %p\n", commands[0], commands[1]);
			printf("index %d\n", index);
			printf("size %d\n", size);

    		//If argument is not the first command
    		if(i > 0){
      			
      			//Hook up stdin
      			dup2(pipes[i-1][0], 0);

      			//Close old FDs 
      			close(pipes[i-1][0]);
      			close(pipes[i-1][1]);
    		}
    
    		//Hook up stdout
    		dup2(pipes[i][1],1);

    		//Close old FDs
    		close(pipes[i][1]);
    		close(pipes[i][0]);


    		// Look for '<' or '>' right here to change commands[i]
    		// find count = numArguments & command index
    		
    		
    		//int index = (commands[i] - commands[0] )/ sizeof(char*);
    		//int size = (commands[i+1] - commands[i] - sizeof(char*) ) / sizeof(char*);
    		endCarrots(index, args, size);



    		//Execute and handle errors if the function returns
    		execvp(commands[i][0], commands[i]);
    		perror(commands[i][0]);
    		exit(1);
    	}
  	}	
 
  	//Handle Last Command
  	pid_t pid = fork();
  	if(pid) { // parent
    	if(pipeCount > 0){
        	close(pipes[pipeCount-1][0]);
        	close(pipes[pipeCount-1][1]);
    	}
    	waitpid(pid, NULL, 0);
  	} 
  	else { // child

    	//If there was a pipe, collect last output
    	if(pipeCount > 0){

        	dup2(pipes[pipeCount-1][0], 0);
        	close(pipes[pipeCount-1][0]);
        	close(pipes[pipeCount-1][1]);
    	}

	    // Handle '<' and '>'
	    // make
    	endCarrots(lastCommand, args, count - lastCommand);
    	
   		//Execute and handle errors if the function returns
    	execvp(args[lastCommand], &args[lastCommand]);
    	perror(args[lastCommand]);
    	exit(1);
    	
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
