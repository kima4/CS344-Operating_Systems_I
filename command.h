/*
* Alexander Kim, kima4
* CS344 - Assignment 3
*
* This file contains the header code for reading and processing commands from the shell interface
* and also defines the command struct.
*/

#ifndef COMMAND_H
#define COMMAND_H


#define ARG_NUM 512 // max number of arguments per the rubric
#define MAX_LEN 2048 // max length of command line per the rubric

/*----------------------------------------------------------------------
*
*  struct command
* -------------
*  Contains information about a command taken from the shell command
*  line.
*
* -------------
*
*  name: a string (char*) that contains the name of the command
*
*  args: an array of strings (char**) that list all of the arguments
*			used to run the command, including the command itself -
*			does not include input/output filenames or background
*			process symbol
*
*  input: a string (char*) that contains the location of a file to
*			be read from for input redirection
*
*  output: a string (char*) that contains the location of a file to
*			be written to for output redirection
*
*  background: an integer, where 0 means that the command is to be
*				run in the foreground and 1 means the command is to
*				be run in the background
*
*---------------------------------------------------------------------*/
struct command {
	char* name;
	char* args[ARG_NUM];
	char* input;
	char* output;
	int background;
};


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
int argType(char* arg);


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
void freeCommand(struct command* toFree);


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
struct command* parseCommand(char* commandLine);


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
void printCommand(struct command* toPrint);


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
char* varExpansion(char* string);

#endif