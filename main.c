/*
* Alexander Kim, kima4
* CS344 - Assignment 3
*
* This is my submission for the third assignment for Operating Systems, an implementation of
* a simple shell in C. It contains three built in commands: cd, status, and exit. To perform 
* other commands, it forks a child and runs the commands with execvp. The shell also supports
* blank lines and comments, expansion of $$ into the shell PID, input and output redirection,
* and running processes in the background. It also handles SIGINT signals such that the shell
* and background processes are not interupted, and SIGTSTP signals, which toggle on and off
* foreground-only mode.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h> // for isspace()

#include "command.h"


volatile sig_atomic_t fgOnly = 0; // foreground only, 0 = no, 1 = yes

/*----------------------------------------------------------------------
*
*  struct pid
* -------------
*  Contains the PID of a process running in the background and the
*  capability to create a linked list for easy removal and traversal
*  through the PID list.
*
* -------------
*
*  num: a pid_t that contains the PID of a process running in the
*		background
* 
*  next: a pointer to a pid struct that represents the next pid struct
*			in the linked list
*
*---------------------------------------------------------------------*/
struct pid {
	pid_t num;
	struct pid* next;
};


/*----------------------------------------------------------------------
*
*  inputRedirect
* -------------
*  Redirects stdin to the specified file. 
*
*  Fulfills requirement 6 of the assignment, in conjunction with 
*  outputRedirect(), by redirecting input from standard input to a
*  specified file.
*
* -------------
*
*  input: a string (char*) that contains the address of the file to
*			be read
*
*  Returns 1 if the input cannot be redirected and prints a message
*  with the error, returns 0 if successful.
*
*---------------------------------------------------------------------*/
int inputRedirect(char* input) {
	int sourceFD;
	int tryDup2;


	sourceFD = open(input, O_RDONLY);
	if (sourceFD == -1) {
		printf("cannot open %s for input\n", input);
		fflush(stdout);
		exit(1);
	}

	tryDup2 = dup2(sourceFD, 0);
	if (tryDup2 == -1) {
		perror("input dup2()");
		exit(1);
	}

	return 0;

}


/*----------------------------------------------------------------------
*
*  outputRedirect
* -------------
*  Redirects stdout to the specified file. Creates the file if it does
*  not exist, truncates it if it does. 
*
*  Fulfills requirement 6 of the assignment, in conjunction with 
*  inputRedirect(), by redirecting output from standard output to a
*  specified file.
*
* -------------
*
*  input: a string (char*) that contains the address of the file to
*			be written to
*
*  Returns 1 if the output cannot be redirected and prints a message
*  with the error, returns 0 if successful.
*
*---------------------------------------------------------------------*/
int outputRedirect(char* output) {
	int targetFD;
	int tryDup2;


	targetFD = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0640);
	if (targetFD == -1) {
		perror("output open()");
		exit(1);
	}

	tryDup2 = dup2(targetFD, 1);
	if (tryDup2 == -1) {
		perror("output dup2()");
		exit(1);
	}

	return 0;

}


/*----------------------------------------------------------------------
*
*  foreground
* -------------
*  Runs the given command in the foreground using a child process and
*  execvp.
* 
*  Fulfills requirement 5 of the assignment by using fork(), an exec()
*  function, and waitpid() to create a child to run commands that are
*  not built in.
* 
*  Fulfills requirement 7 of the assignment, in conjunction with
*  background() and checkBackground(), by running commands as
*  foreground processes.
* 
*  Fulfills requirement 8 of the assignment, in conjunction with
*  getCommand(), by enabling the foreground processes to be interupted 
*  by a SIGINT signal.
*
* -------------
*
*  c: a command struct that is to be executed
* 
*  status: a string (char*) that contains the status of the last
*			command run in the foreground, which is to be overwritten
*
*  Returns a string (char*) containing the status of the command
*  once it has terminated, either naturally or forced.
*
*---------------------------------------------------------------------*/
char* foreground(struct command* c, char* status) {
	pid_t newPid = fork();
	int childStatus;

	switch (newPid) {
	case -1:
		perror("fork()\n");
		exit(1);
		break;

	case 0: // child
		signal(SIGINT, SIG_DFL); // revert to default behavior for SIGINT handling
		if (c->input != NULL) {
			inputRedirect(c->input);
		}
		if (c->output != NULL) {
			outputRedirect(c->output);
		}
		execvp(c->name, c->args);

		perror(c->name);
		exit(1);
		break;

	default: // parent
		newPid = waitpid(newPid, &childStatus, 0);
		if (WIFEXITED(childStatus)) {
			sprintf(status, "exit value %d", WEXITSTATUS(childStatus));
			return status;
		}
		else {
			sprintf(status, "terminated by signal %d", WTERMSIG(childStatus));
			printf("%s\n", status); // print the required termination message
			fflush(stdout);
			return status;
		}

	}
	
}


/*----------------------------------------------------------------------
*
*  background
* -------------
*  Runs the given command in the background using a child process and
*  execvp. 
*
*  Fulfills requirement 5 of the assignment, in conjunction with
*  checkBackground(), by using fork() and an exec() function to
*  create a child to run commands that are not built in.
* 
*  Fulfills requirement 7 of the assignment, in conjunction with
*  foreground() and checkBackground(), by running commands as 
*  background processes.
*
* -------------
*
*  c: a command struct that is to be executed
*
*  Returns the PID of the child process running in the background.
*
*---------------------------------------------------------------------*/
pid_t background(struct command* c) {
	pid_t newPid = fork();

	switch (newPid) {
	case -1:
		perror("fork()\n");
		exit(1);
		break;

	case 0: // child
		if (c->input != NULL) {
			inputRedirect(c->input);
		}
		else {
			inputRedirect("/dev/null");
		}
		if (c->output != NULL) {
			outputRedirect(c->output);
		}
		else {
			inputRedirect("/dev/null");
		}

		execvp(c->name, c->args);

		perror(c->name);
		exit(1);
		break;

	default: // parent
		printf("background pid is %d\n", newPid);
		fflush(stdout);
		return newPid;

	}

}


/*----------------------------------------------------------------------
*
*  checkBackground
* -------------
*  Iterates through a list of PIDs for background processes and checks
*  each to see if any have finished.
*
*  Fulfills requirement 5 of the assignment, in conjunction with
*  background(), by using waitpid() to determine if a process has
*  finished.
* 
*  Fulfills requirement 7 of the assignment, in conjunction with 
*  foreground() and background(), by printing a message showing the
*  PID and exit status of background processes that have terminated.
*
* -------------
*
*  head: a pid struct that acts as the head of the linked list of pid
*			structs
*
*  Prints exit status messages for completed background processes. 
*  Returns the head of the linked list with completed processes
*  removed.
*
*---------------------------------------------------------------------*/
struct pid* checkBackground(struct pid* head) {
	struct pid* curr = head;
	struct pid* prev = NULL; // for connecting linked list parts when a link is removed
	pid_t childPid;
	int childStatus;

	while (curr != NULL) {
		childPid = waitpid(curr->num, &childStatus, WNOHANG); // don't wait, just check
		if (childPid != 0) {
			printf("background pid %d is done: ", childPid);
			fflush(stdout);
			if (WIFEXITED(childStatus)) {
				printf("exit value %d\n", WEXITSTATUS(childStatus));
			}
			else {
				printf("terminated by signal %d\n", WTERMSIG(childStatus));
			}
			fflush(stdout);

			// remove completed pid structs
			if (prev == NULL) {
				head = head->next;
				curr = head;
			}
			else {
				prev->next = curr->next;
				free(curr);
				curr = prev->next;
			}
			
		}
		else {
			prev = curr;
			curr = curr->next;
		}
	}
	return head;

}


void SIGTSTP_off(int signum); // to prevent implicit declaration


/*----------------------------------------------------------------------
*
*  SIGTSTP_on
* -------------
*  Signal handler for SIGTSTP, turns foreground-only mode on. Adapted
*  from a post on Ed Discussion.
* 
*  Fulfills requirement 8 of the assignment, in conjunction with 
*  SIGTSTP_off(), by allowing the user to toggle background-only mode
*  on with a SIGTSTP signal.
*
* -------------
*
*  signum: int that represents the type of signal handled - should
*			really only be 20
*
*  Returns nothing, but enables foreground-only mode. Also changes
*  SIGTSTP handling so that the next SIGTSTP disables foreground-only
*  mode.
*
*---------------------------------------------------------------------*/
void SIGTSTP_on(int signum) {
	fgOnly = 1;
	write(STDOUT_FILENO, "\nEntering foreground-only mode (& is now ignored)\n: ", 52);
	signal(SIGTSTP, &SIGTSTP_off); // next SIGTSTP turns off foreground-only mode
}


/*----------------------------------------------------------------------
*
*  SIGTSTP_off
* -------------
*  Signal handler for SIGTSTP, turns foreground-only mode off. Adapted
*  from a post on Ed Discussion.
* 
*  Fulfills requirement 8 of the assignment, in conjunction with
*  SIGTSTP_on(), by allowing the user to toggle background-only mode
*  off with a SIGTSTP signal.
*
* -------------
*
*  signum: int that represents the type of signal handled - should
*			really only be 20
*
*  Returns nothing, but disables foreground-only mode. Also changes
*  SIGTSTP handling so that the next SIGTSTP enables foreground-only
*  mode.
*
*---------------------------------------------------------------------*/
void SIGTSTP_off(int signum) {
	fgOnly = 0;
	write(STDOUT_FILENO, "\nExiting foreground-only mode\n: ", 32);
	signal(SIGTSTP, &SIGTSTP_on); // next SIGTSTP turns on foreground-only mode
}


/*----------------------------------------------------------------------
*
*  isBlank
* -------------
*  Determines if the given string has a command or if it just a blank
*  line or comment.
*
*  Fulfills requirement 2 of the assignment by informing the shell
*  when to ignore lines due to them being blank or comments.
*
* -------------
*
*  str: a string (char*) to be processed
*
*  Returns 0 if the given line is a command, returns 1 if it should be
*  ignored.
*
*---------------------------------------------------------------------*/
int isBlank(char* str) {
	if (str[0] == '#') { // check if comment
		return 1;
	}

	while (*str != '\0') { // check if blank
		if (!isspace((unsigned char)*str)) {
			return 0;
		}
		str++;
	}
	return 1;
}


/*----------------------------------------------------------------------
*
*  buildInCD
* -------------
*  Code for the built in change directory command for the shell.
*
*  Fulfills requirement 4 of the assignment, in conjunction with
*  getCommand(), by providing a built in cd command for the shell.
*
* -------------
*
*  cd: a command struct that can contain an address as the second
*		argument
*
*  Changes directory to the specified directory or the home directory
*  if one is not given. Returns the exit signal of chdir - currently
*  unused.
*
*---------------------------------------------------------------------*/
int builtInCD(struct command* cd) {
	char* newDir;
	int exitSig;

	if (cd->args[1] == NULL) {
		newDir = getenv("HOME");
	}
	else {
		newDir = cd->args[1];
	}

	exitSig = chdir(newDir);

	return exitSig;
}


/*----------------------------------------------------------------------
*
*  getCommand
* -------------
*  The backbone of the shell, runs continuously until an exit command
*  is recieved.
* 
*  Fulfills requirement 4 of the assignment, in conjunction with
*  builtInCD(), by implementing actions for built-in status and exit 
*  commands.
* 
*  Fulfills requirement 8 of the assignment by ignoring SIGINT signals, 
*  which is inherited by background processes as well, and by setting
*  the default SIGTSTP behavior such that the first SIGTSTP signal 
*  turns background-only mode on.
*
* -------------
* 
*  Returns 0 when the exit command is given.
*
*---------------------------------------------------------------------*/
int getCommand() {
	char* status = malloc(MAX_LEN);
	struct command* c;

	// linked list of background process PIDs
	struct pid* head = NULL;
	struct pid* tail = NULL;

	sprintf(status, "exit value 0"); // default status for before any foreground processes are run

	signal(SIGTSTP, &SIGTSTP_on); // first SIGTSTP turns background only mode on


	while (1) {
		signal(SIGINT, SIG_IGN); // reset SIGINT behavior to ignore for shell

		head = checkBackground(head); // check for completed background processes

		char commandLine[MAX_LEN];

		printf(": "); // prompt for command line
		fflush(stdout);


		fgets(commandLine, MAX_LEN, stdin); // get command from user input
		if ((strlen(commandLine) > 0) && (commandLine[strlen(commandLine) - 1] == '\n')) {
			commandLine[strlen(commandLine) - 1] = '\0'; // removes newline inserted by fgets()
		}

		if (!isBlank(commandLine)) {
			c = parseCommand(commandLine);
			if (strcmp(c->name, "exit") == 0) { // built in exit command
				free(status);
				return 0;
			}
			else if (strcmp(c->name, "cd") == 0) { // built in cd command
				builtInCD(c);
			}
			else if (strcmp(c->name, "status") == 0) { // built in status command
				printf("%s\n", status);
				fflush(stdout);
			}
			else if (c->background == 0 || fgOnly == 1) { // run in foreground
				status = foreground(c, status);

			}
			else { // run in background
				struct pid* newPid = malloc(sizeof(struct pid));
				newPid->num = background(c);

				// add to or create linked list of pids
				if (head == NULL) {
					head = newPid;
				}
				else {
					tail->next = newPid;
				}
				tail = newPid;
			}
			freeCommand(c);
		}

		commandLine[0] = '\0'; // clear previous command line

	}
	

}


/*----------------------------------------------------------------------
*
*  main
* -------------
*  Just here for moral support.
*
* -------------
*
*  Returns 0 on exit.
* 
*---------------------------------------------------------------------*/
int main() {
	getCommand();
	return 0;
}