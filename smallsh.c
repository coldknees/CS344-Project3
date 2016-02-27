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
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>


#define MAX_ARGS 512
#define MAX_LENGTH 2048

// Function Prototypes
void shLoop();
void shEx(char **commands, int numCom, struct sigaction *saCh, int *stat);
int runCommand(char**);
int isBackground(char**);
void clearBuff(char*);
void clearArr(char**);
void interrupt(int);
int numArgs(char **args);



volatile sig_atomic_t stop;

int main(int argc, char * argv[]) {	


	shLoop();

	return 0;	
}


void shLoop() {


	// Set up signal handler
	struct sigaction sa, saCh;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	
	saCh.sa_handler = SIG_DFL;
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, &saCh);

	//Files for io
	FILE *in;
	FILE *out;


	int status = 0;
	int bgstatus, waitStat;
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
		pid_t bgwait;
		bgwait = waitpid(-1, &bgstatus, WNOHANG);
		if(WIFEXITED(bgstatus) && bgwait > 0) {
			printf("background pid %d is done: exit value %d\n", bgwait, WEXITSTATUS(bgstatus));
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

		//conting number of arguments
		int num = 0;
		while(commandArr[num] != NULL) {
			printf("%d", num);
			num++;
		}

		//check for built in commands
		if(valid == 1) {
			if(strcmp(commandArr[0], "exit") == 0) {
				printf("exiting...\n");
				cont = 0;
			}

			else if(strcmp(commandArr[0], "cd") == 0) {
				// if no path specified change to home
				printf("cd command recognized");
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
					printf("Exit Status: %d", status);
				}
				else if(WIFSIGNALED(status)) {
					printf("terminated by signal%d\n", WTERMSIG(status));
				}
			}
			else {
				shEx(commandArr, num, &saCh, &status);
			}
		}

		free(commandBuff);
		free(commandArr);

	} while(cont);

}


void shEx(char **commands, int numCom, struct sigaction *saCh, int *stat) {
	pid_t spawnPid, exitPid;
	int bgProc = 0;
	int out, in, f1, f2;

	
	if (spawnPid == 0) {
		// check if the process should be run in the background, set flag
		if(strcmp(commands[numCom-1], "&") == 0) {
			bgProc = 1;
		}
	}

	spawnPid = fork();
	if(spawnPid == 0) {
		if(bgProc != 1) {
			sigaction(SIGINT, saCh, NULL);
		}
	

		// handle input and output redirection case
		if(numCom >= 3) {
			if(strcmp(commands[1], "<") == 0) {
				f2 = open(commands[2], O_RDONLY);
				if(f2 == -1) {
					perror("reading");
					exit(1);
				}
				in = dup2(f2, 0);
				if(in == -1) {
					perror("dup");
					exit(1);
				}

				// free arg[1]
			}
		}
		else if(strcmp(commands[1], ">") == 0) {
			f1 = open(commands[2], O_WRONLY);
			if(f1 == -1) {
				perror("writing");
				exit(1);
			}
			out = dup2(f1, 1);
			if(out == -1) {
				perror("dup2");
				exit(1);
			}		
		}
		execvp(commands[0], commands);
		perror(commands[0]);
		exit(1);

	}
	else if(spawnPid > 0) {
		//background
		if(bgProc == 1) {
			printf("background pid is %d", spawnPid);
		}
		//foreground
		else {
			exitPid = waitpid(spawnPid, stat, 0);
			if(exitPid == -1) {
				perror("fg wait");
			}
			else if (exitPid > 0) {
				if(WIFEXITED(*stat)) {
					printf("terminated by signal %d\n", WTERMSIG(*stat), exitPid);
				}
			}
		}			
	}
	else {
		printf("fork failed\n");
	}

	// check background
	int bgstatus;
	pid_t bgwait;
	bgwait = waitpid(-1, &bgstatus, WNOHANG);
	if(WIFEXITED(bgstatus) && bgwait > 0) {
		printf("background pid %d is done: exit value %d\n", bgwait, WEXITSTATUS(bgstatus));
	}
	else if(WIFSIGNALED(bgstatus) && bgwait > 0) {
		int sig;
		sig = WTERMSIG(bgstatus);
		printf("terminated by signal %d\n", sig);
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