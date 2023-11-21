#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFLEN 1024
#define MAX_ARGS 10
int argc;

// Feature 2
// to tokenize parsedinput based on spaces while 
// still respecting quotation marks
void tokenize(char *input, char *args[]) {

    // First tokenize with respect to space marks
    char *token = strtok(input, " ");
    int i = 0;
    int inQuotes = 0;

    // Iterate through input tokens (separated in respect to spaces) 
    // while there is still space in args[]
    while (token != NULL && i < MAX_ARGS) {

        // If token starts and ends with a "
        if ((token[0] == '"') && (token[strlen(token) - 1] == '"')) {
            // We aren't in quotes after this token, just handle this token as
            // quotes and move on without quotes
            inQuotes = 0;
            // Allocate mem and add the token to args[i]
            args[i] = malloc((strlen(token) + 1) * sizeof(char));
            strcpy(args[i], token);
        }
        // If the first character of the token is "
        else if (token[0] == '"') {
            // We are in quotes, ignore spaces
            inQuotes = 1;
            // Allocate mem and add the token to args[i]
            args[i] = malloc((strlen(token) + 1) * sizeof(char));
            strcpy(args[i], token + 1); 
        } 
        // Else if the last character of the token is "
        else if(token[strlen(token) - 1] == '"') {
            // We aren't in quotes anymore
            inQuotes = 0;
            // Append a " " to the existing token and add it to args[i]
            strcat(args[i], " ");
            strncat(args[i], token, strlen(token) - 1);
        }
        // If the token is not enclosed in quotes
        else {
            // If we are inside quotes, append to args[i] (current args element)
            if (inQuotes) {
                strcat(args[i], " ");
                strcat(args[i], token);
            } 
            // Else we aren't inside quotes, 
            // allocate new mem and store in new args[i]
            else {
                args[i] = malloc((strlen(token) + 1) * sizeof(char));
                strcpy(args[i], token);
            }
        }
        // If we aren't in quotes, increment i
        if (!inQuotes) {
            i++;
        }
        //Get the next token
        token = strtok(NULL, " ");
    }
    // If there are still empty elements after this, set them to NULL
    argc = i;
    while (i < MAX_ARGS) {
        args[i] = NULL;
        i++;
    }
}

int main() {
    char buffer[BUFLEN];
    char* parsedinput;
    char* args[MAX_ARGS + 1];  // Add 1 for NULL

    pid_t completed_pid;

    printf("Welcome to the GroupXX shell! Enter commands, enter 'quit' to exit\n");
    do {    
        // Check for completed background processes (Feature 5)
        while ((completed_pid = waitpid(-1, NULL, WNOHANG)) > 0) {
            printf("Background command %d terminated\n", completed_pid);
        }

        //Print the terminal prompt and get input
        printf("$ ");
        char *input = fgets(buffer, sizeof(buffer), stdin);
        if (!input) {
            fprintf(stderr, "Error reading input\n");
            return -1;
        }

        //Clean and parse the input string
        parsedinput = (char*) malloc(BUFLEN * sizeof(char));
        size_t parselength = trimstring(parsedinput, input, BUFLEN);

        // Tokenize the input string (Feature 2)
        tokenize(parsedinput, args);

        // Feature 5
        while ((completed_pid = waitpid(-1, NULL, WNOHANG)) > 0) {
            printf("Background command %d terminated\n", completed_pid);
        }
        if (args[argc-1] != NULL && strcmp(args[argc -1], "&") == 0) {
            // Remove the "&" from the arguments
            args[argc -1] = NULL;

            pid_t child_pid = fork();
            if (child_pid == 0) {
                // Child process:
                if (execvp(args[0], args) == -1) {
                    fprintf(stderr, "Error running command in execvp\n");
                    return -100;
                }
            } else if (child_pid > 0) {
                // Parent process: don't wait
            } 
        }
        else {

            // Feature 3
            char *command = args[0];
            int completed = 0;
            int isExecutable = 0;

            // If the command starts with a '/'
            if(command[0] == '/') {
                // We can get our args[0] from here, don't need to use PATH
                completed = 1;
            }

            // If the command doesn't start with a '/'
            else {
                // Check if it can be run as an executable
                // Set executable to args[0] prepended with "./"
                char* executable = malloc(strlen(command) + 3); // +3 because 2 for ./ and one for null terminator
                strcpy(executable, "./");
                strcat(executable, command);

                // If the executable is accessible
                if(access(executable, X_OK) != -1) {
                    // Set args[0] as the executable
                    args[0] = executable;
                    // Args is completed and there is an executable
                    completed = 1;
                    isExecutable = 1;
                }
            }

            // If there is no executable and it doesn't start with a '/'
            if((isExecutable == 0) && (completed == 0)) {
                // Get environment variable path and tokenize
                char *path = getenv("PATH");
                // Mkae a copy as to not modify the original
                char *path_tokenize = strdup(path);
                char *token = strtok(path_tokenize, ":");

                // While there are still tokens available
                while(token != NULL) {
                    char command_path[256];
                    strcat(command_path, token);
                    strcat(command_path, "/");
                    strcat(command_path, args[0]);

                    // If the path matches and accessible
                    if(access(command_path, X_OK) == 0) {
                        // Set args[0] as the path found in PATH
                        args[0] = command_path;
                        break;
                    }
                    // Get the next token
                    token = strtok(NULL, ":");
                }
            }


            // Check if the command contains a pipe
            int hasPipe = 0;
            char* leftCommand[MAX_ARGS + 1] = {NULL};
            char* rightCommand[MAX_ARGS + 1] = {NULL};

            // Iterate through args
            for (int k = 0; args[k] != NULL; k++) {

                // If args[k] is "|"
                if (strcmp(args[k], "|") == 0) {

                    // There is a pipe
                    hasPipe = 1;

                    // Set the rest of leftCommand to null (move on to right)
                    int l = k;
                    while(l < MAX_ARGS) {
                        leftCommand[l] = NULL;
                        l++;
                    }
                    // Iterate through the rest of args and set them to the rightCommand
                    int j;
                    for (j = 0; args[k + j + 1] != NULL; j++) {
                        rightCommand[j] = malloc((strlen(args[k + j + 1]) + 1) * sizeof(char));
                        strcpy(rightCommand[j], args[k + j + 1]);
                    }
                    // Set the rest of rightCommand to null
                    while(j < MAX_ARGS) {
                        rightCommand[j] = NULL;  // Null-terminate right command
                        j++;
                    }
                    // Completed separation, break
                    break;
                }
                // Else add args[k] to leftCommand
                else{
                    leftCommand[k] = malloc((strlen(args[k]) + 1) * sizeof(char));
                    strcpy(leftCommand[k], args[k]);
                }
            }

            // If there is a pipe
            if (hasPipe) {

                // Feature 4 declarations
                int pipe_fd[2];
                pipe(pipe_fd);

                // First fork
                pid_t child1 = fork();
                
                // If it's a child
                if (child1 == 0) {
                    // Child 1
                    // Close read end of pipe
                    close(pipe_fd[0]);  
                    // Send stdout to write end of pipe
                    dup2(pipe_fd[1], STDOUT_FILENO); 
                    // Close write end of pipe
                    close(pipe_fd[1]); 

                    // Run the first command
                    if (execvp(leftCommand[0], leftCommand) == -1) {
                        fprintf(stderr, "Error running command in execvp\n");
                        return -100;
                    }
                } 
                else {
                    // Second fork
                    pid_t child2 = fork();

                    // If it's a child
                    if (child2 == 0) {
                        // Child 2
                        // Close the write end of the pipe
                        close(pipe_fd[1]);  
                        // Send stdin to the read end of the pipe
                        dup2(pipe_fd[0], STDIN_FILENO);  
                        // Close the read end of the pipe
                        close(pipe_fd[0]);

                        // Run the second command
                        if (execvp(rightCommand[0], rightCommand) == -1) {
                            fprintf(stderr, "Error running command in execvp\n");
                            return -100;
                        }
                    } 
                    
                    else {
                        // This is the parent, close pipe
                        close(pipe_fd[0]);
                        close(pipe_fd[1]);

                        wait(NULL);
                        wait(NULL);
                        for (int j = 0; j < MAX_ARGS; j++) {
                            free(leftCommand[j]);  
                        }
                        for (int j = 0; j < MAX_ARGS; j++) {
                            free(rightCommand[j]);  
                        }
                    }
                }
            } 
            // If there is no pipe, execute normally
            else {
                //Sample shell logic implementation
                if (strcmp(parsedinput, "quit") == 0) {
                    printf("Bye!!\n");
                    return 0;
                } else {
                    pid_t forkV = fork();
                    if (forkV == 0) {
                        if (execvp(args[0], args) == -1) {  // Use execvp to search PATH
                            fprintf(stderr, "Error running command in execvp\n");
                            return -100;
                        }
                    } else {
                        wait(NULL);
                    }
                }
            }
        }
        
        //Remember to free any memory you allocate!
        free(parsedinput);
        // Free allocated memory for args
        for (int j = 0; j < MAX_ARGS; j++) {
            free(args[j]);  
        }

    } while (1);

    return 0;
}