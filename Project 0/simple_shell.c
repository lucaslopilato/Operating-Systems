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



char* specialToken[3] = { "<" , ">" , "|" };
int indeces[8] = {0,0,0,0,0,0,0,0};


void endCarrots(int indexOfCommand, char** args, int size){ 

	int index = size - 1;
	while(index >= indexOfCommand){
	    // handle "<" function process
	    if(strcmp(args[index],specialToken[0])==0) {
		    int fd;
		    fd = open(args[index+1], O_RDONLY);
		    // Check if file exists
		    if(fd<0) 
		        perror("No such file or directory\n");
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
	int j = 0;
  	for (int i = 0; i<count; i++){
   		//Handle "|" function process 
    	if(strcmp(args[i],specialToken[2])==0) {
     		//Create New Pipe
	        if(pipe(pipes[pipeCount]) < 0) perror("Pipe Creation Failed");
      		pipeCount++;
		// indeces of first and last of each subarray
		j = j+2;
		indeces[j-1]= i - 1;
		indeces[j]= i + 1;
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
	indeces[j+1]=count-1;	
	//Put last command in command
	commands[pipeCommands] = &args[lastCommand];
  	//Ensure Proper Amount of Pipes
  	if(pipeCount > 3) perror("Greater than 3 pipes Requested \n");
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
	                int index = indeces[i*2];
			int size = indeces[(i*2)+1] - indeces[i*2];
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
	    int lastarg = indeces[(pipeCount*2)+1];
    	endCarrots(lastCommand, args, lastarg);
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
}
