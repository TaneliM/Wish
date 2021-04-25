**DESCRIPTION**
A simple c program that mimics the functionality of a Unix shell.


**USAGE**
./wish
Interactive mode - Runs manually typed commands in the terminal.

./wish batch.txt
Batch mode - Runs the commands given in a file then exits.


**FEATURES**
The shell calls execv function to execute the commands given by the user.


BUILT IN COMMANDS
There are 3 built in commands: exit, path, cd.

exit - exits the program
usage:
wish> exit

path - Defines a custom path to executables to run them as commands (by default: /bin)
usage:
wish> path /path/to/directory/with/executables

cd - change working directory
usage:
wish> cd /path/to/new/'working directory'


**TESTING**
tests can be found at: https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/processes-shell
