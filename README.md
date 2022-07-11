# CS344-Operating_Systems_I
Portfolio project for CS344 at Oregon State University


The purpose of this project was to write a simple shell in C. The program had eight requirements for functionality:
1. Provide a prompt for running commands
    - A colon `:` was used as a prompt for each command line
    - The general syntax of a command line was `command [arg1 arg2 ...] [< input file] [> output file] [&]`
    - The shell did not need to support piping or quotation marks
    - The shell needed to support lines up to 2048 characters and 512 arguments
    - No error checking for sytax was required

2. Handle comments and blank lines
    - Lines beginning with `#` needed to be treated as comments and ignored
    - Lines with no input needed to be ignored

3. Provide expansion for the variable $$
    - Any instance of `$$` needed to be expanded into the PID of smallsh itself

4. Execute three built-in commands
    - exit - exits the shell and takes no arguments
    - cd - changes the working directory of smallsh
    - status - prints out the exit status or terminating signal of the last foreground process

5. Execute other commands by creating new processes using a function from the `exec` family
    - smallsh needed to fork off a child and use an `exec` function to run any non-built-in commands
    - The child needed to terminate once finished running the command

6. Support input and output redirection
    - The `<` and `>' symbols needed to indicate input and output redirection, respectively

7. Support running commands in foreground and background processes
    - Any command without `&` at the end needed to be run in the foreground, with smallsh waiting for completion before prompting for the next command
    - Any command with `&` at the end needed to be run in the background by forking a child, with smallsh immediately prompting for the next command

8. Implement custom handlers for two signals, SIGINT and SIGTSTP
    - smallsh and children running as background processes needed to ignore SIGINT, while a child running as a foreground process needed to terminate and the parent needed to print the signal number
    - All child processes needed to ignore SIGTSTP and smallsh needed to alternate between states of ignoring `&` and not ignoring `&` arguments upon receiving SIGTSTP
    
    
Result: 180/180
