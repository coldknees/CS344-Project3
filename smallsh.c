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
#include <sys/wait.h>
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
void interrupt(int);
int numArgs(char **args);

//int for status
int exitStat;

volatile sig_atomic_t stop;

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

	//Files for io
	FILE *in;
	FILE *out;


	int status, bgstatus, wait, bgProc;
	int cont = 1;
	pid_t bgwait;

	do {

		// Allocate memory to read in the command
		char *commandBuff = malloc(MAX_LENGTH);
		char **commandArr = malloc(MAX_ARGS * sizeof(char*));

		// set all of the pointers to null
		int count;
		for(count = 0; count < MAX_ARGS; count++) {
			commandArr[count] = NULL;
		}
		// declare necessary varibles
		int valid = 1;

		//checks for status of background processes
		wait = waitpid(-1, &bgstatus, WNOHANG);
		if(WIFEXITED(bgstatus) && bgwait > 0) {
			printf("background pid %d is done: exit value %d\n", wait, WEXITSTATUS(bgstatus));
		}
		else if(WIFSIGNALED(bgstatus) && bgwait > 0) {
			int sig;
			sig = WTERMSIG(bgstatus);
			printf("terminated by signal %d\n", sig);
		}


		// Get the command fromt the user
		printf(": ");
		fflush(stdout);
		fgets(commandBuff, MAX_LENGTH, stdin);
		

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

		//check for built in commands
		if(valid == 1) {
			if(strcmp(commandArr[0], "exit") == 0) {
				printf("exiting...\n");
				cont = 0;
			}

			else if(strcmp(commandArr[0], "cd") == 0) {
				// if no path specified change to home
				if(commandArr[1] == NULL) {
					char *home = getenv("HOME");
					chdir(home);
				}
				else if(chdir(commandArr[1]) == -1) {
					printf("no such directory\n");
				}
			}
			else if(strcmp(commandArr[0], "status") == 0) {
				if(WIFEXITED(bgstatus)) {
					printf("Exit Status: ");
				}
			}
		}

		printf("before check for redirection\n");



		int num = 0;
		while(commandArr[num] != NULL) {
			printf("%d", num);
			num++;
		}
		printf("There are %d arguments\n", num);


		// if there is more than one command, check for file io
		if(num > 1 && valid == 1) {
			int ioFlag;
			// check for io redirection
			if(strcmp(commandArr[1], "<") == 0) {
				ioFlag = 1;
				if(!(in = fopen(commandArr[2], "r"))) {
					printf("error opening file");
				}
			}
			else if(strcmp(commandArr[1], ">") == 0) {
				ioFlag = 2;
				if(!(out = fopen(commandArr[2], "w"))) {
					printf("error opening file");
				}
			}
		}

		if(valid == 1) {
			//signal handler
			signal(SIGINT, interrupt);

			// testing this signal interrupt
			if(strcmp(commandArr[0], "testing") == 0) {
				while(!stop) {
					pause();
				}
				printf("SIGINT sucessfully handled\n");
			}
		}

		// check if the process should be run in the background, set flag
		//bgProc = isBackground(commandArr);


		// No built in function detected, fork process and run command
		//pid_t childPid, childWait;
		//childPid = fork();




		free(commandBuff);
		free(commandArr);

	} while(cont);

	

}

// checks if the command should be ran in the background
// numCommands is an index
int isBackground(char **commands) {
	int numCommands = 0;
	while(commands[numCommands] != NULL) {
		numCommands++;
	}
	if(strcmp(commands[numCommands], "&") == 0) {
		commands[numCommands] = NULL;
		return 1;
	}
	else {
		return 0;
	}
}

void interrupt(int sig) {
	stop = 1;
}

int numArgs(char **args) {
	int num = 0;
	while(args[num] != NULL) {
		num++;
	}
	return num;
}