// Ben Azran 208276162
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

typedef struct commandInfo {
    char command[100];
    char* argv[100];
    pid_t pid;
    int isBackground; // 0 - foreground, 1 - background
} commandInfo;

commandInfo commandsInfo[100];

void commandInfoInit(commandInfo* command, char* userInput){
    // Initialize argv with NULLs
    memset(command->argv, NULL, 100);

    // Initialize command->command with NULLs
    memset(command->command, NULL, 100);

    // Copy the input from userInput to the struct
    strcpy(command->command, userInput);


    // Initialize isBackground
    command->isBackground = 0;

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
        p2 = p1 + strlen(command->command) + 1;
    }

    // If the last argument is '&' - it's background
    if(strcmp("&", command->argv[argvIndex - 1]) == 0){
        command->isBackground = 1;
        command->argv[argvIndex - 1] = NULL;
    }
}

void jobs(commandInfo* commandsInfo, int numberOfCommands){
    
}

int main() {
    char userInput[100];
    int commandIndex = 0;

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
            //jobs(commandsInfo, commandIndex);
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
            // Background
            if (commandsInfo[commandIndex].isBackground == 0){
                waitResult = waitpid(commandsInfo[commandIndex].pid, NULL, 0);
                printf("Wait: %d\n", waitResult);
            }
            printf("Pid: %d\n", pid);
        }

        commandIndex++;
    }
}
