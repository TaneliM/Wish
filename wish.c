#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


void defaultError(void) {
	fprintf(stderr, "Error occurred\n");
}


// Storing the command into an array of arguments
char** parseCommand(char* command) {
	char** arguments;
	char* tmp;
	int i = 0;
	
	if ((arguments = malloc(sizeof(char *))) == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		return NULL;
	}
	tmp = strtok(command, " ");
	
	// Storing the arguments into the array one by one
	while (tmp != NULL) {
		arguments[i] = tmp;
		
		// reallocating memory for the array to fit the next argument
		if ((arguments = realloc(arguments, sizeof(char*) * (i + 1))) == NULL) {
			fprintf(stderr, "Memory allocation failed\n");
			return NULL;
		}
		
		i++;
		tmp = strtok(NULL, " ");
	}
	arguments[i] = NULL;
	return arguments;
}


// Using execvp to execute the command
int executeCommand(char* command) {
	char** arguments;
	if ((arguments = parseCommand(command)) == NULL) {
		return -1;
	}
	
	// TODO custom commands: exit, path
	// TODO child process
	if ((execvp(arguments[0], arguments)) == -1) {
		defaultError();
	}
	free(arguments);
	return 0;
}


// reads a line from stdin and removes '\n'
char* readCommand(void) {
	char* command;
	size_t n = 0;
	command = NULL;
	
	if (getline(&command, &n , stdin) >= 0) {
		if (command == NULL) {
			fprintf(stderr, "Error while reading command\n");
		}
		else { // TODO Fix crash from 0 length command
			command[strlen(command) - 1] = '\0';
    	}
	}
	return command;
}


int main(int argc, char** argv) {
	char* command;
	FILE* inputStream = stdin;
	
	switch (argc) {
		case 1: // Interactive mode
			while (1) {
				fprintf(stdout, "wish> ");
				command = readCommand();
				executeCommand(command);
				free(command);
			}
			break;
		
		case 2: // Batch mode
			// TODO read from file
			break;
		
		default: // Too many arguments
			fprintf(stderr, "Usage: './wish' or './wish batch.txt'\n");
			exit(1);
	}
	
	return 0;
}

