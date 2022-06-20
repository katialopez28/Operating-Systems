/*
    Name: Katia Lopez
    Shell Assignment
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11    // Requirement 7: Your version of Mav shell shall
                                // support up to 10 command line parameters in
                                // addition to the command.



int main()
{
    char *cmd_str = (char*) malloc( MAX_COMMAND_SIZE );


    // Create an array to store history of commands
    char *history[15];
    for (int i=0; i<15; i++)
    {
        history[i] = (char*)malloc(255);
    }
    int history_index = 0;

    // need to keep track of number of history commands
    // to know when to stop printing history array
    int hist_counter = 0;


    // Create an array of PIDs to later use listpids
    pid_t pid_list[15];
    int pid_index = 0;

    // need to keep track of number of processes
    // to know when to stop printing pid array
    int pid_counter = 0;


    // Requirement 4: After each command completes, your program
    // shall print the msh> prompt and accept another line of input.
    // Using while(1) so prompt continues printing until user types
    // quit or exit
    while( 1 )
    {
        // Requirement 1: Your program will print out a prompt
        // of msh> when it is ready to accept input
        printf ("msh> ");

        // Read the command from the commandline.  The
        // maximum command that will be read is MAX_COMMAND_SIZE
        // This while command will wait here until the user
        // inputs something since fgets returns NULL when there
        // is no input
        while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

        // Requirement 6: If the user types a blank line, your shell
        // will, quietly and with no other output, print another prompt
        // and accept a new line of input.
        if (cmd_str[0] == '\n' || cmd_str[0] == ' ' || cmd_str[0] == '\t')
        {
            // will skip the rest of the code and return to top of
            // while loop statement to print prompt again
            continue;
        }


        // For the ! command: right after the fgets call immediately
        // check for the ! and if it�s a ! command then over write
        // the user input with the correct history command.

        if (cmd_str[0] == '!')
        {
            // convert the number after the ! to an int to be able
            // to use as an index for history
            char *working_str  = strdup(cmd_str);
            char *number = strtok(working_str, "!");

            //need to convert to an int to be able to use it as an index
            //for history array
            int nth_command = atoi(number);

            // If the nth command does not exist then your shell will
            // state "Command not in history."
            if (nth_command >= hist_counter)
            {
                printf("Command not in history\n");
                continue;
            }
            else
            {
                // Do search and replacement. Overwrite cmd_str with history
                // then continue so command gets executed again
                strncpy(cmd_str, history[nth_command], 255);
            }
        }


        /* Parse input */
        char *token[MAX_NUM_ARGUMENTS];
        int   token_count = 0;

        // Pointer to point to the token
        // parsed by strsep
        char *argument_ptr;

        char *working_str  = strdup( cmd_str );


        // use memset() to clear out your input string
        // before using it
        memset(history[history_index], 0, 255);

        // Copy command from working string to history array
        strncpy(history[history_index++], working_str, 255);

        // Once history_index reaches 14 that means there are
        // 15 history commands in the array, so change index to 0
        // Will rewrite over the array
        if(history_index > 14)
        {
            history_index = 0;
            hist_counter = 15;
        }
        else if (hist_counter > 15)
        {
            // history counter cannot be greater than the
            // size of the array
            hist_counter = 15;
        }
        else
        {
            //have less than 15 commands so increment counter
            hist_counter++;
        }

        // we are going to move the working_str pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *working_root = working_str;

        // Tokenize the input strings with whitespace used as the delimiter
        while ( ( (argument_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
        {
            token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
            if( strlen( token[token_count] ) == 0 )
            {
                token[token_count] = NULL;
            }
            token_count++;
        }



        // Will check what command user has entered based on token array
        if (strcmp(token[0], "cd") == 0)
        {
            // Requirement 10: Your shell shall support the cd command
            // to change directories. Your shell must handle cd ..
            if(token[1] != NULL)
            {
                chdir(token[1]);
            }
            else
            {
                printf("Invalid directory\n");
            }
        }
        else if (strcmp(token[0], "history") == 0)
        {
            // Requirement 12: Your shell shall support the history command
            // which will list the last 15 commands entered by the user.

            int index = history_index; //oldest one
            fflush(NULL);

            //double checking so index 15 does not get printed
            if (hist_counter > 15)
            {
                // history counter cannot be greater than the
                // size of the array
                hist_counter = 15;
            }

            for (int i=0; i < hist_counter; i++)
            {
                printf("%d: %s\n", i, history[i]);

                if (index>14)
                {
                    index = 0;
                }
            }
        }
        else if (strcmp(token[0], "listpids") == 0)
        {
            /* Requirement 11: Your shell shall support the listpids command
            to list the PIDs of the last 15 processes spawned by your shell.
            If there have been less than 15 processes spawned then it shall
            print only those process PIDs */

            // will print the array pid_list which stores the last n processes
            int i;
            int n = pid_counter;
            for (i=0; i<n; i++)
            {
                printf("%d: %d\n", i, pid_list[i]);
            }
        }
        else if (strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0)
        {
            // Requirement 5: Your shell will exit with status zero
            // if the command is "quit" or "exit".
            exit(0);
        }
        else
        {
            pid_list[pid_index++] = fork();


            // Once pid_index reaches 14 that means there are
            // 15 processes in the array, so change index to 0
            // Will rewrite over the array
            if(pid_index > 14)
            {
                pid_index = 0;
                pid_counter = 15;
            }
            else if (pid_counter > 15)
            {
                // pid counter cannot be greater than the
                // size of the array
                pid_counter = 15;
            }
            else
            {
                // have less than 15 processes so increment counter
                pid_counter++;
            }


            int temp_index = pid_index - 1;

            if (pid_list[temp_index] == 0) //we are in the child process
            {
                // Requirement 3: If the command option is an invalid option then your
                // shell shall print the command followed by ": invalid option --" and
                // the option that was invalid as well as a prompt to try �help.
                // exec() outputs this automatically so use execvp
                int ret = execvp(token[0], &token[0]);

                // Requirement 2: If the command is not supported your shell shall
                // print the invalid command followed by ": Command not found."
                if (ret == -1)
                {
                    printf("%s: Command not found\n", token[0]);
                    fflush(NULL);
                    exit(EXIT_SUCCESS);
                }
                else
                {
                    fflush(NULL);
                    exit(EXIT_SUCCESS);
                }
            }
            else //in parent process
            {
                int status;

                // Force the parent process to wait until the child process exits
                waitpid(pid_list[temp_index], &status, 0 );
                fflush( NULL );
            }
    }

    free(working_root);
  }

  return 0;
}
