/***********************************************************************
* Mark Rushmere
* CS 344
* Project 3
* Description: This is a small shell script program with support for the
* commands exit, cd, and status.
***********************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define MAX_ARGS 512
#define MAX_LENGTH 2048

// Function Prototypes
void shLoop();
int runCommand(char**);
int isBackground(char**);
void clearBuff(char*);
void clearArr(char**);


int main(int argc, char * argv[]) {	

	// Set up signal handler
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigfillset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);

	shLoop();

	return 0;	
}


void shLoop() {

	//int for status
	int status;

	// Allocate memory to read in the command
	char *commandBuff = malloc(MAX_LENGTH);
	char **commandArr = malloc(MAX_ARGS * sizeof(char*));


	do {
		int valid = 1;

		// Get the command fromt the user
		printf(": ");
		fflush(stdout);
		fgets(commandBuff, MAX_LENGTH,stdin);
		

		//check for comment lines
		if(commandBuff[0] == '#') {
			printf("comment line\n");
			valid = 0;
		}

		//check if the line is empty
		if(commandBuff[0] == '\n') {
			printf("The line was empty\n");
			valid = 0;
		}

		// Parse the commands
		if(valid == 1) {
			int i = 0;
			char* temp;
			strtok(commandBuff, "\n");
			temp = strtok(commandBuff, " \n");
			while(temp != NULL && i <= MAX_ARGS) {
				commandArr[i] = temp;
				i++;
				temp = strtok(NULL, " \n");
			}
		}



		// testing for now, just print them out
		int j;
		for(j = 0; j < 3; j++) {
			printf("%d:  %s\n", j, commandArr[j]);
		}



		//check for exit command
		if(valid == 1) {
			if(strcmp(commandArr[0], "exit") == 0) {
				printf("exiting...\n");
				status = 0;
			}
			else {
				status = runCommand(commandArr);
			}
		}
		
		clearBuff(commandBuff);
		clearArr(commandArr);

	} while(status);

	free(commandBuff);
	free(commandArr);

}

int runCommand(char **commands) {


	if(isBackground(commands)) {
		printf("this is a background command");
	}

	pid_t pid, wpid;
	pid_t spawnpid = -5;
	int status;

	spawnpid = fork();


	switch(spawnpid) {
		case -1:
			printf("Something went wrong with the fork!!\n");
			exit(1);
			break;
		case 0:
			printf("child...\n");
			if(execvp(commands[0], commands) == -1) {
				perror("lsh");
			}

		default: 
			printf("i am the parent");
			do {
				wpid = waitpid(pid, &status, WUNTRACED);
			} while(!WIFEXITED(status) && !WIFSIGNALED(status));

	}


	//check for a cd command
	if(strcmp(commands[0], "cd") == 0)   {
		printf("changing directories....\n");
	}


	// check for ls command
	if(strcmp(commands[0], "ls") == 0) {
		printf("listing current dir...\n");
	}

	return 1;
}

// Sources Cited
// http://brennan.io/2015/01/16/write-a-shell-in-c/

void clearBuff(char *buffer) {
	printf("clearing buffer\n");
	memset(buffer, 0, (sizeof(char)*MAX_LENGTH));
}

void clearArr(char **arr) {
	printf("clearing array\n");
	int i;
	for(i = 0; i < MAX_ARGS; i++) {
		arr[i] = NULL;
	}
}


// checks if the command should be ran in the background
int isBackground(char **commands) {
	int numCommands = 0;
	while(commands[numCommands] != NULL) {
		numCommands++;
	}
	if(strcmp(commands[numCommands], "&") == 0) {
		return 1;
	}
	else {
		return 0;
	}
}