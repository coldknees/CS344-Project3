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



int main(int argc, char * argv[]) {	

	// Set up signal handler
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigfillset(%sa.sa_mask);
	sigaction(SIGINT, &act, NULL);

	// Allocate memory to read in the command
	char *commandBuff = malloc(MAX_LENGTH);
	char **commandArr = malloc(MAX_ARGS * sizeof(char*));

	// main loop continues until the user enters "exit"
	while(strcmp(commandBuff, "exit") != 0) {

		
		// Get the command fromt the user
		printf(": ");
		fflush(stdout);
		fgets(commandBuff, MAX_LENGTH,stdin);
		strtok(commandBuff, "\n");

		// Parse the commands
		int i = 0;
		char* temp;
		temp = strtok(commandBuff, " \n");
		while(temp != NULL && i <= MAX_ARGS) {
			commandArr[i] = temp;
			temp = strtok(commandBuff, " \n");
		}

		// testing for now, just print them out
		int j;
		for(j = 0; j < i; j++) {
			printf("%d:  %s", j, commandArr[j]);
		}
	}

	free(commandBuff);
	free(commandArr);
}





void runCommand() {

}
