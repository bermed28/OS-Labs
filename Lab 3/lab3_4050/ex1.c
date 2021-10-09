#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define TOKEN_DELIMETERS " \t\r\n\a"
#define DEFAULT_SIZE 64

char *history[DEFAULT_SIZE];
char *argv[DEFAULT_SIZE];
int hist_count = 0;
int hist_num = 0;
int position = 0;
short isHistNum = 0;

void read_line(char *line){
    if(isHistNum){
        line = memcpy(line,history[hist_num-1],DEFAULT_SIZE);
        printf("\n");
    }
    else{
        gets(line);
    }
    isHistNum = 0;
    memcpy(history[hist_count],line,DEFAULT_SIZE);
    hist_count++;
}

void parse_line(char *line,char *argv[]){
    char *token;
    position = 0;
    token = strtok(line,TOKEN_DELIMETERS);
    while(token!=NULL){
        argv[position] = token;
        position++;
        token =strtok(NULL,TOKEN_DELIMETERS);
    }
}

int nat_history(char *argv[]){
    if(position==2){
        hist_num = atoi(argv[1]);
        for(int k = 1; k<=hist_count;k++){
            if(hist_num == k){
                isHistNum = 1;
            }
        }
    }
    if(isHistNum==0){
        for(int i =0; i <hist_count;i++)
            printf(" %d %s\n",(i+1),history[i]);
    }
    return 1;
}

void help(){
    printf("Shell Basic Commands\n");
    printf("cd <path> - Change Directory to the one specified in <path>\n");
    printf("history - Show list of previous commands executed\n");
    printf("history <n> - Execute the nth command in the history\n");
    printf("ls - Show the contents of the current directory\n");
    printf("ls -l - Show the contents of the current directory with more detail\n");
    printf("exit - Terminate the current CLI\n");
}

void execute(char *argv[]){

    // Check if command is valid as a native command
    char exit_[10];    strcpy(exit_, "exit");
    char hist[10]; strcpy(hist, "history");
    char cd[10]; strcpy(cd, "cd");
    char help_[10]; strcpy(help_, "help");

    //Native commands
    if(argv[0] != NULL) {
        if (strcmp(argv[0], exit_) == 0) {
            exit(0);
        } else if (strcmp(argv[0], hist) == 0) {
            nat_history(argv);
        } else if (strcmp(argv[0], cd) == 0) {
            if(argv[1] == NULL)
                chdir("/");
            else
                chdir(argv[1]);
        } else if (strcmp(argv[0], help_) == 0) {
            help();
        } else {
            //Execute any other command. Uses child process.
            pid_t pid = fork();
            if (pid == -1) {
                printf("ERORR on fork\n");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                execvp(argv[0], argv);
                exit(0);
            } else {
                while(waitpid(-1, NULL, 0) > 0);
            }
        }
    }
}

int main(int argc, char *argv[]){
    int valid = 0;
    char *line = (char*)malloc(DEFAULT_SIZE);
    for(int i = 0;i<DEFAULT_SIZE;i++)
        history[i] = (char*)malloc(DEFAULT_SIZE);
    long size;
    char *buf;
    char *ptr;
    bzero(argv, DEFAULT_SIZE); //Clear argument array
    while(1){
        printf("Shell->");
        read_line(line);
        parse_line(line,argv);
        execute(argv);
        for(int j = 0; j < argc;j++){
            argv[j] = NULL;
        }
    }
}
