// Ben Azran 208276162
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct commandInfo {
    char command[100];
    char* argv[100];
    pid_t pid;
    int isBackground; // 0 - foreground, 1 - background
} commandInfo;

commandInfo commandsInfo[100];
char oldPath[100];

void commandInfoInit(commandInfo* command, char* userInput){
    // Initialize argv with NULLs
    memset(command->argv, NULL, 100);

    // Initialize command->command with NULLs
    memset(command->command, NULL, 100);

    // Copy the input from userInput to the struct
    strcpy(command->command, userInput);

    // Initialize isBackground
    command->isBackground = 0;

    // Initialize pid
    command->pid = -1;

    /*
     * Insert NULLs instead of every space character
     * Token the userInput
     */

    // Split every 'space'
    char delim[] = " ";

    // Split the first token
    char *ptr = strtok(command->command, delim);

    // Split all tokens
    while (ptr != NULL) {
        // Split the command
        ptr = strtok(NULL, delim);
    }

    // At this point command->command contains the userInput with every space character replaced with NULL
    char* p1;
    char* p2;

    p1 = command->command;
    p2 = p1 + strlen(command->command) + 1;

    int argvIndex = 0;

    while(*p1 != '\0'){
        command->argv[argvIndex] = p1;
        argvIndex++;
        p1 = p2;
        p2 = p1 + strlen(p1) + 1;
    }

    // If the last argument is '&' - it's background
    if(strcmp("&", command->argv[argvIndex - 1]) == 0){
        command->isBackground = 1;
        command->argv[argvIndex - 1] = NULL;
    }
}

void jobs(int numberOfCommands){
    pid_t waitResult;
    int argvIndex;

    // Iterate over all of the commands
    for(int i = 0; i < numberOfCommands; i++){
        // Case the current command isn't a background command - skip it
        if(commandsInfo[i].isBackground == 0)
            continue;

        argvIndex = 0;

        // Check using WNOHANG if the background command is still running
        waitResult = waitpid(commandsInfo[i].pid, NULL, WNOHANG);

        // Case 'waitpid' returned 0 - the background command is still running
        if(waitResult == 0){
            // Print the command name
            printf("%s", commandsInfo[i].argv[0]);
            fflush(stdout);

            argvIndex = 1;

            // Print the command arguments
            while(commandsInfo[i].argv[argvIndex] != NULL){
                printf(" %s", commandsInfo[i].argv[argvIndex]);
                fflush(stdout);
                argvIndex++;
            }

            printf("\n");
        }
    }
}

void history(int numberOfCommands){
    pid_t waitResult;
    int argvIndex;
    char* currentCommand;

    // Iterate over all of the commands
    for(int i = 0; i < numberOfCommands; i++){
        currentCommand = commandsInfo[i].command;

        argvIndex = 0;

        // Case the current command is a 'built-in' command - mark as 'DONE'
        if(strcmp(currentCommand, "jobs") == 0 || strcmp(currentCommand, "cd") == 0
            || strcmp(currentCommand, "exit") == 0 || (strcmp(currentCommand, "history") == 0 && i != numberOfCommands - 1)){
            waitResult = 1;
        }

       // Check using WNOHANG if the command is still running
       else{
            waitResult = waitpid(commandsInfo[i].pid, NULL, WNOHANG);
       }

        // Print the command name
        printf("%s", commandsInfo[i].argv[0]);
        fflush(stdout);

        argvIndex = 1;

        // Print the command arguments
        while(commandsInfo[i].argv[argvIndex] != NULL){
            printf(" %s", commandsInfo[i].argv[argvIndex]);
            fflush(stdout);
            argvIndex++;
        }

        // Case 'waitpid' returned 0 - the command is still running
        if(waitResult == 0)
            printf(" RUNNING\n");

        // Otherwise, the command is done
        else
            printf(" DONE\n");
    }

    // The current command is history and of course it's still running
    printf("history RUNNING\n");
}

void changeDirectory(int chdirIndex){
    char* cdParam = commandsInfo[chdirIndex].argv[1];

    // Case 'cd' was inserted with more than one argument - print error
    if(commandsInfo[chdirIndex].argv[2] != NULL){
        printf("Too many argument\n");
        return;
    }

    int status;
    char* homePath = getenv("HOME");
    char currentPath[100];
    memset(currentPath, NULL, 100);

    // Save the current Path
    getcwd(currentPath, 100);

    // Case 'cd' has one argument or 'cd ~' was inserted - return home
    if(cdParam == NULL || strcmp("~", cdParam) == 0){
        status = chdir(homePath);
        if(status == -1)
            printf("An error occurred\n");
        else
            // Update the old path
            strcpy(oldPath, currentPath);
        return;
    }

    char destPath[100];
    memset(destPath, NULL, 100);
    int index;

    // Case the path start with a '~'
    if(cdParam[0] == '~') {
        // Replace '~' with home path
        strcpy(destPath, homePath);

        index = strlen(homePath);

        // Concatenates the path after the home path
       for(int i = 1; i < strlen(cdParam); i++){
           destPath[index] = cdParam[i];
            index++;
        }

        // Change dir
        status = chdir(destPath);
    }

    // Case the path start with a '-'
    else if(cdParam[0] == '-'){
        // Case the command wasn't 'cd -' it's not valid
        if(cdParam[1] != '\0'){
            printf("An error occurred\n");
            return;
        }

        // Case it's the first time cd is used - no old path
        if(oldPath[0] == '\0'){
            printf("An error occurred\n");
            return;
        }

        status = chdir(oldPath);
    }

    // Any other case
    else{
        status = chdir(cdParam);
    }

    // Case chdir failed
    if(status == -1)
        printf("chdir failed\n");

    // Case chdir haven't failed - save the old path
    else
        // Update the old path
        strcpy(oldPath, currentPath);
}

int main() {
    char userInput[100];
    int commandIndex = 0;
    memset(oldPath, NULL, 100);

    while(1){
        // Display Prompt
        printf("$ ");
        fflush(stdout);

        // Scan a command
        scanf(" %[^\n]", userInput);

        // Initialize the first command info in the commandsInfo array
        commandInfoInit(&commandsInfo[commandIndex], userInput);

        int status;
        pid_t pid;
        int execStatus;
        pid_t waitResult;

        // jobs command
        if(strcmp("jobs", userInput) == 0){
            jobs(commandIndex);
            commandIndex++;
            continue;
        }

        // history command
        if(strcmp("history", userInput) == 0){
            history(commandIndex);
            commandIndex++;
            continue;
        }

        // cd command
        if(strcmp("cd", commandsInfo[commandIndex].command) == 0){
            changeDirectory(commandIndex);
            commandIndex++;
            continue;
        }

        // Create fork
        pid = fork();

        if(pid != 0 && pid != -1)
            commandsInfo[commandIndex].pid = pid;

        // Fork Failure
        if (pid == -1)
            printf("fork failed\n");

        // Child
        else if (pid == 0) {
            execStatus = execvp(commandsInfo[commandIndex].argv[0], commandsInfo[commandIndex].argv);

            if (execStatus == -1)
                printf("exec failed\n");
        }

        // Father
        else {
            // Foreground
            if (commandsInfo[commandIndex].isBackground == 0){
                waitResult = waitpid(commandsInfo[commandIndex].pid, NULL, 0);
            }
        }

        commandIndex++;
    }
}
