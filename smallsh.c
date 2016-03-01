/***********************************************************************
* Mark Rushmere
* CS 344
* Project 3
* Description: This is a small shell script program with built in 
* support for the commands exit, cd, and status. processes can be ran
* in the background by having "&" as the last argument. 
* To exit the program type "exit".
***********************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>


#define MAX_ARGS 512
#define MAX_LENGTH 2048

// Function Prototypes
void shLoop();
void shEx(char **commands, int numCom, struct sigaction *saCh, struct sigaction *sa, int *stat);
int runCommand(char**);
int isBackground(char**);
void checkBackground();
void clearBuff(char*);
void clearArr(char**);


int main(int argc, char * argv[]) {	

	shLoop();
	return 0;	
}


void shLoop() {


	// Set up signal handler
	struct sigaction sa, saCh;
	// sa sig handler is the sigHandle function
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	
	// background processes get default signal handling
	saCh.sa_handler = SIG_DFL;
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, &saCh);

	//Files for io
	FILE *in;
	FILE *out;


	// initialize forground and background exit statuses
	int status = 0;
	int bgstatus;
	int cont = 1;
	

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
		checkBackground();

		// Get the command fromt the user
		printf(": ");
		fflush(stdout);
		fgets(commandBuff, MAX_LENGTH, stdin);
		

		//check for comment lines
		if(commandBuff[0] == '#') {
			valid = 0;
		}

		//check if the line is empty
		if(commandBuff[0] == '\n') {
			valid = 0;
		}

		// Parse the commands
		if(valid == 1) {
			int i = 0;
			char* temp;
			strtok(commandBuff, "\n");
			//split the line up with spaces or newlines
			temp = strtok(commandBuff, " \n");
			while(temp != NULL && i <= MAX_ARGS) {
				commandArr[i] = temp;
				i++;
				temp = strtok(NULL, " \n");
			}
		}

		//conting number of arguments
		int num = 0;
		while(commandArr[num] != NULL) {
			num++;
		}

		//check for built in commands
		if(valid == 1) {
			if(strcmp(commandArr[0], "exit") == 0) {
				cont = 0;
			}

			else if(strcmp(commandArr[0], "cd") == 0) {
				// if no path specified change to home
				if(commandArr[1] == NULL) {
					char *home = getenv("HOME");
					chdir(home);
				}
				// if the directory is entered wrong. Print error message
				else if(chdir(commandArr[1]) == -1) {
					printf("no such directory\n");
				}
			}
			// status command
			else if(strcmp(commandArr[0], "status") == 0) {
				if(WIFEXITED(status)) {
					printf("exit value %d\n", WEXITSTATUS(status));
				}
				else if(WIFSIGNALED(status)) {
						printf("terminated by signal %d\n", WTERMSIG(status));
				}
			}
			else {
				shEx(commandArr, num, &saCh, &sa, &status);
			}
		}

		// remove buffer and array
		free(commandBuff);
		free(commandArr);

	} while(cont);

}


void shEx(char **commands, int numCom, struct sigaction *saCh, struct sigaction *sa, int *stat) {
	pid_t spawnPid, exitPid;
	int bgProc = 0;
	int f2, f1;
	int in = -7;
	int out = -7;

	// check if the process should be run in the background, set flag
	if(strcmp(commands[numCom-1], "&") == 0) {
		bgProc = 1;
		commands[numCom-1] = NULL;
	}

	spawnPid = fork();
	if(spawnPid == 0) {

		// set the handlers for sigints
		if(bgProc != 1) {
			sigaction(SIGINT, saCh, NULL);
		}
		//while loop to go through the arguments
		int argCount = 0;
		while(commands[argCount] != NULL) {
			// handle input and output redirection case
			if(strcmp(commands[argCount], "<") == 0) {
				f1 = open(commands[argCount+1], O_RDONLY);
				if(f1 == -1) {
					perror("cannot open badfile or input\n");
					exit(1);
				}
				in = dup2(f1, 0);
				if(in == -1) {
					perror("dup2\n");
					exit(1);
				}
				commands[argCount] = NULL;

			}

			else if(strcmp(commands[argCount], ">") == 0) {
				f2 = open(commands[argCount+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if(f2 == -1) {
					perror("cannot open badfile or input\n");
					exit(1);
				}
				out = dup2(f2, 1);
				if(out == -1) {
					perror("dup2\n");
					exit(1);
				}
				commands[argCount] = NULL;
			}
			argCount++;
		}

		// execute the commands using the array of commands
		execvp(commands[0], commands);
		perror(commands[0]);
		exit(1);


	}
	else if(spawnPid > 0) {
		//background
		if(bgProc == 1) {
			printf("background pid is %d\n", spawnPid);
		}
		//foreground
		else {
			exitPid = waitpid(spawnPid, stat, 0);
			if((exitPid > 0) && WIFSIGNALED(*stat)) {
				printf("terminated by signal %d\n", WTERMSIG(*stat), exitPid);
			}			
		}			
	}
	else {
		printf("process failed\n");
	}

	checkBackground();
}


// checks background processes
void checkBackground() {
		pid_t bgp = 0;
		int bgs;
		do {
			bgp = waitpid(-1, &bgs, WNOHANG);
			if (bgp > 0) {
				if(WIFEXITED(bgs)) {
					printf("background pid %d is done: exit value %d\n", (int)bgp, WEXITSTATUS(bgs));
				}
				else if(WIFSIGNALED(bgs)) {
					printf("terminated by signal %d\n", WTERMSIG(bgs));
				}
			}
		} while(bgp > 0);
}