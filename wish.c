#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


// Maximum path length on linux
#define MAX_PATH 4096


// default error
static const char error_message[30] = "An error has occurred\n";


// struct for a linked list
struct node {
	char* string;
	struct node * prev;
	struct node * next;
} typedef Node;


int main(int argc, char** argv);
char* readCommand(void);
Node* readBatch(Node* commands, FILE* stream);
int executeCommand(char* command, Node* paths);
char* checkPaths(char* path, Node* paths, char* command);
char** parseCommand(char* command);
int builtInCommands(Node* paths, char** arguments);
void pathCombine(char* path, char** arguments);
Node* addNode(Node** list, char* str);
void deleteList(Node* list);
void debugPrintList(Node* list);


// Creating a new node to the list and giving it a string value
Node* addNode(Node** list, char* str) {
	Node* pCurrent = NULL;
	pCurrent = *list;
	
	if (*list == NULL) { 
		// memory allocation for the first node
		if ((*list = pCurrent = malloc(sizeof(Node))) == NULL) {
			fprintf(stderr, "malloc failed\n");
			return NULL;
		}
		pCurrent->prev = pCurrent->next = NULL;
	} else {
		// Finding the last node
		while (pCurrent->next != NULL) {
			pCurrent = pCurrent->next;
		}
		
		// Memory allocation for a new last node
		if ((pCurrent->next = (Node*)malloc(sizeof(Node))) == NULL) {
			fprintf(stderr, "malloc failed\n");
			return NULL;
		}
		
		// Linking the node to the list
		pCurrent->next->prev = pCurrent;
		pCurrent = pCurrent->next;
		pCurrent->next = NULL;
	}
	// Memory allocation for a string in the new node
	if ((pCurrent->string = malloc(strlen(str) + 1)) == NULL) {
		fprintf(stderr, "malloc failed\n");
		return NULL;
	}
	
	// Storing the string into the node
	strcpy(pCurrent->string, str);
	return *list;
}


// Printing the list
void debugPrintList(Node * list) {
	Node* pCurrent;
	pCurrent = list;
	
	// Going through the linked list while printing
	while (pCurrent->next != NULL) {
		fprintf(stdout, "%s\n", pCurrent->string);
		pCurrent = pCurrent->next;	
	}
	fprintf(stdout, "%s\n", pCurrent->string);
}


// Deleting the linked list
void deleteList(Node* list) {
	Node* pCurrent;
	pCurrent = list;
	
	// Freeing allocated memory
	while (pCurrent != NULL) {
		list = pCurrent->next;
		free(pCurrent->string);
		free(pCurrent);
		pCurrent = list;
	}
}


// Combining the path from arguments and adding spaces
void pathCombine(char* path, char** arguments) {
	int i = 1;
	
	while (arguments[i] != NULL) {
		strcat(path, arguments[i]);
		i++;
		if (arguments[i] != NULL) {
			strcat(path, " ");
		}
	}
}


// Handling built in commands
int builtInCommands(Node* paths, char** arguments) {
	char path[MAX_PATH] = "\0";
	pathCombine(path, arguments);
	
	// exit command
	if (strcmp(arguments[0], "exit") == 0) {
		if (arguments[1] == NULL) {
			return 2;
		}
		else {
			write(STDERR_FILENO, error_message, strlen(error_message));
			return 1;
		}
		
	}
	// cd command
	else if (strcmp(arguments[0], "cd") == 0) {
		
		if (chdir(path) == -1) {
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
		return 1;
	}
	// path command
	else if (strcmp(arguments[0], "path") == 0) {
	
		/* The exercise description calls for multiple paths to be accessible
		however the prewritten tests for the program require the path to be overwritten.
		This is the reason why the paths are stored in a linked list with a length of 1. */
		free(paths);
		paths = NULL;
		paths = addNode(&paths, path);
		return 1;
	}
	else {
		return 0;
	}
}


// Storing the command into "char** arguments"
char** parseCommand(char* command) {
	char** arguments;
	char** tmpArguments;
	char* tmp;
	int i = 0;
	
	if ((arguments = malloc(sizeof(char *))) == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		return NULL;
	}
	tmp = strtok(command, " ");
	
	// Storing the arguments into the char** one by one
	while (tmp != NULL) {
		arguments[i] = tmp;
		
		// reallocating memory for the char** to fit the next argument
		if ((tmpArguments = realloc(arguments, sizeof(char*) * (i + 2))) == NULL) {
			fprintf(stderr, "Memory allocation failed\n");
			return NULL;
		}	
		else {
			arguments = tmpArguments;
		}
		
		i++;
		tmp = strtok(NULL, " ");
	}
	arguments[i] = NULL;
	return arguments;
}


// Searching for a valid path to the executable from the defined paths
char* checkPaths(char* path, Node* paths, char* command) {
	Node* pCurrent = paths;
	
	// Adding the executable name to the path
	while (pCurrent != NULL) {
		strcpy(path, "\0");
		strcat(path, pCurrent->string);
		strcat(path, "/");
		strcat(path, command);
		
		// Trying to access the executable
		if (access(path, X_OK) == 0) {
			return path;
		}
		pCurrent = pCurrent->next;
	}
	return NULL;
}


// Using execv to execute the command in a chils process
int executeCommand(char* command, Node* paths) {
	char** arguments;
	char path[MAX_PATH] = "\0";
	pid_t pid;
	int rv;
	
	// Separating the arguments
	if ((arguments = parseCommand(command)) == NULL) {
		return -1;
	}
	
	// Checking for built in command
	if ((rv = builtInCommands(paths, arguments)) == 1) {
		return 0;
	} 
	else if (rv == 2) {
		return 2;
	}
	else {
		// Creating a child process to execute the command
		switch (pid = fork()) {
			case -1:
				fprintf(stderr, "Fork failed.");
				return -1;
				
			case 0:; // child process
				if ((checkPaths(path, paths, arguments[0])) == NULL) {
					;
				}
				
				// Executing the command
				if ((execv(path, arguments)) == -1) {
					write(STDERR_FILENO, error_message, strlen(error_message));
				}
				
				free(command);
				free(arguments);
				deleteList(paths);
				exit(0);
				
			default: // parent process waits for child to finish
				wait(NULL);
				break;
		}
	}
	free(arguments);
	return 0;
}


// Reading input to the list
Node* readBatch(Node* commands, FILE* stream) {
	char* buffer = NULL;
	size_t n = 0;
		
	// Reading file line by line
	// Using getline() to read a string of characters of undefined length: https://stackoverflow.com/questions/29576799/reading-an-unknown-length-line-from-stdin-in-c-with-fgets
	while (getline(&buffer, &n , stream) >= 0) {
		if (buffer == NULL) {
			fprintf(stderr, "getline failed\n");
			return NULL;
		}
		else { 
			// easy way to get rid of a trailing newline: https://aticleworld.com/remove-trailing-newline-character-from-fgets/
			buffer[strlen(buffer) - 1] = '\0';
    	}
		// Storing commands to a list
		commands = addNode(&commands, buffer);
		
	}
	free(buffer);
	return commands;
}


// Reading a line from stdin and removing the trailing newline character
char* readCommand(void) {
	char* command;
	size_t n = 0;
	command = NULL;
	
	// Using getline() to read a string of characters of undefined length: https://stackoverflow.com/questions/29576799/reading-an-unknown-length-line-from-stdin-in-c-with-fgets
	if (getline(&command, &n , stdin) >= 0) {
		if (command == NULL) {
			fprintf(stderr, "Error while reading command\n");
		}
		else { 
			// easy way to get rid of a trailing newline: https://aticleworld.com/remove-trailing-newline-character-from-fgets/
			command[strlen(command) - 1] = '\0';
    	}
	}
	return command;
}


int main(int argc, char** argv) {
	Node* paths = NULL;
	char* command;
	char* defaultPath = "/bin";
	
	// Adding default path to paths list
	if ((paths = addNode(&paths, defaultPath)) == NULL) {
		exit(1);
	}
	
	switch (argc) {
		case 1: // Interactive mode
			// Reading commands one by one and passing them to executeCommand to run them
			while (1) {
				fprintf(stdout, "wish> ");
				command = readCommand();
				
				if (!strcmp(command, "") || !strcmp(command, " ")) {
					continue;
				}
				
				switch (executeCommand(command, paths)) {
					case -1:
						write(STDERR_FILENO, error_message, strlen(error_message));
						free(command);
						break;
						
					case 2:
						free(command);
						deleteList(paths);
						exit(0);
						
					default:
						free(command);
						break;
				}
			}
			break;
		
		case 2: ;// Batch mode
			Node* commands = NULL;
			Node* pCurrent = NULL;
			FILE* batchFile;
			
			// Reading commands from a file to a linked list
			if ((batchFile = fopen(argv[1], "r")) == NULL) {
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(1);
			}
			
			if ((commands = readBatch(commands, batchFile)) == NULL) {
				write(STDERR_FILENO, error_message, strlen(error_message));
				deleteList(paths);
				exit(1);
			}
			
			fclose(batchFile);
			
			pCurrent = commands;
			
			// Executing the commands one by one
			while (pCurrent != NULL) {
				executeCommand(pCurrent->string, paths);
				pCurrent = pCurrent->next;
			}
			
			deleteList(commands);
			break;
		
		default: // Too many arguments
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(1);
	}
	deleteList(paths);
	return 0;
}

