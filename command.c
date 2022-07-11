/*
* Alexander Kim, kima4
* CS344 - Assignment 3
*
* This file contains the code for reading and processing commands from the shell interface
* and creating a command struct, which is defined in command.h, that contains the information
* needed to perform the command.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "command.h"


/*----------------------------------------------------------------------
*
*  freeCommand
* -------------
*  Frees all necessary attributes of a struct to prevent memory leaks.
*
* -------------
*
*  toFree: a pointer to a command struct that is to be freed
*
*  Returns nothing, but frees all attributes of a struct that are not
*  NULL and then frees the struct.
*
*---------------------------------------------------------------------*/
void freeCommand(struct command* toFree) {
	int i = 0;
	free(toFree->name);
	while (toFree->args[i] != NULL) {
		free(toFree->args[i++]);
	}
	if (toFree->input != NULL) {
		free(toFree->input);
	}
	if (toFree->output != NULL) {
		free(toFree->output);
	}
	free(toFree);
}



/*----------------------------------------------------------------------
*
*  printCommand
* -------------
*  Prints the values of each attribute in a given command struct for
*  testing purposes.
*
* -------------
*
*  toprint: a pointer to a command struct that is to be printed
*
*  Returns nothing, but prints the information contained in a command
*  struct, separated by attribute.
*
*---------------------------------------------------------------------*/
void printCommand(struct command* toPrint) {
	int i = 0;

	printf("COMMAND: %s\n\n", toPrint->name);

	printf("ARGS:\n");
	while (toPrint->args[i] != NULL) {
		printf("|  %s\n", toPrint->args[i++]);
	}
	printf("\n");

	printf("INPUT FILE: %s\n\n", toPrint->input);

	printf("OUTPUT FILE: %s\n\n", toPrint->output);

	printf("BACKGROUND: ");
	if (toPrint->background == 1) {
		printf("YES\n\n");
	}
	else {
		printf("NO\n\n");
	}
}


/*----------------------------------------------------------------------
*
*  argType
* -------------
*  Checks the given argument to determine if it is a regular argument
*  or if it has some special meaning.
*
*  Fulfills requirement 1 of the assignment, in conjunction with
*  parseCommand(), by identifying special symbols used in the command
*  syntax.
*
* -------------
*
*  arg: a string (char*) that is an argument for the command that is
*		being processed
*
*  Returns an int whose value is dependent on what kind of argument it
*  is:
*		1 - regular arguments
*		2 - symbol declaring the next argument gives an input file
*		3 - symbol declaring the next argument gives an output file
*		4 - ampersand, declares the process runs in the background
*
*---------------------------------------------------------------------*/
int argType(char* arg) {
	if (strcmp(arg, "<") == 0) {
		return 2;
	}
	else if (strcmp(arg, ">") == 0) {
		return 3;
	}
	else if (strcmp(arg, "&") == 0) {
		return 4;
	}
	return 1;
}


/*----------------------------------------------------------------------
*
*  varExpansion
* -------------
*  Modifies a given string such that substrings of "$$" are replaced
*  with the PID of the shell.
*
*  Fulfills requirement 3 of the assignment by expanding the variable
*  "$$" into the PID of the shell itself.
*
* -------------
*
*  string: a string (char*) that is to undergo variable expansion
*
*  Returns the string with all "$$" substrings replaced with the shell
*  PID.
*
*---------------------------------------------------------------------*/
char* varExpansion(char* string) {
	char newString[MAX_LEN];
	char holder[MAX_LEN];
	char* ptr;
	char* tmp;
	pid_t pid = getpid();

	while ((tmp = strstr(string, "$$"))) { // while $$ is still in the string
		ptr = tmp + 2;
		strncpy(holder, string, tmp - string); // copy beginning of string up to $$
		holder[tmp - string] = '\0';
		sprintf(newString, "%s%d%s", holder, pid, ptr); // replace the first instance of $$ with the shell PID
		sprintf(string, "%s", newString);

	}

	return string;
}


/*----------------------------------------------------------------------
*
*  parseCommand
* -------------
*  Converts a command line into a command struct for processing.
*
*  Fulfills requirement 1 of the assignment, in conjunction with
*  argType(), by parsing the command line and splitting it based on
*  the syntax.
*
* -------------
*
*  commandLine: a string (char*) taken from user input on the shell
*				interface
*
*  Returns a command struct that contains all of the information
*  from the command line.
*
*---------------------------------------------------------------------*/
struct command* parseCommand(char* commandLine) {

	struct command* com = malloc(sizeof(struct command));
	int bookmark = 0; // 0 = name, 1 = args, 2 = input, 3 = output, 4 = background
	char* tmp = calloc(MAX_LEN, sizeof(char)); // temporary string holder for variable expansion

	// for strtok_r
	char* tok;
	char* saveptr;

	// for saving arugments to array in command struct
	int argNum = 0;

	// default values
	com->input = NULL;
	com->output = NULL;
	com->background = 0;

	// add the command name to both the name and args attributes
	tok = strtok_r(commandLine, " ", &saveptr);
	strcpy(tmp, tok);
	tmp = varExpansion(tmp);
	com->name = calloc(strlen(tmp) + 1, sizeof(char));
	strcpy(com->name, tmp);
	com->args[argNum] = calloc(strlen(tmp) + 1, sizeof(char));
	strcpy(com->args[argNum++], tmp);

	tok = strtok_r(NULL, " ", &saveptr);
	while (tok != NULL) {

		// if a lone & is added as an argument instead of as a background flag
		if (bookmark == 4) {
			com->background = 0;
			com->args[argNum] = calloc(2, sizeof(char));
			strcpy(com->args[argNum++], "&");
		}


		bookmark = argType(tok);
		if (bookmark == 2) { // argument is "<"
			tok = strtok_r(NULL, " ", &saveptr);
			if (tok != NULL) { // in case nothing is following the "<"
				strcpy(tmp, tok);
				tmp = varExpansion(tmp);
				com->input = calloc(strlen(tmp) + 1, sizeof(char));
				strcpy(com->input, tmp);
			}
		}
		else if (bookmark == 3) { // argument is ">"
			tok = strtok_r(NULL, " ", &saveptr);
			if (tok != NULL) { // in case nothing is following the ">"
				strcpy(tmp, tok);
				tmp = varExpansion(tmp);
				com->output = calloc(strlen(tmp) + 1, sizeof(char));
				strcpy(com->output, tmp);
			}
		}
		else if (bookmark == 4) { // argument is "&"
			com->background = 1;
		}
		else { // argument is generic
			strcpy(tmp, tok);
			tmp = varExpansion(tmp);
			com->args[argNum] = calloc(strlen(tmp) + 1, sizeof(char));
			strcpy(com->args[argNum++], tmp);

		}
		tok = strtok_r(NULL, " ", &saveptr);
	}
	com->args[argNum] = NULL;

	return com;


}
