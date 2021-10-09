#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <sys/un.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/wait.h>

// Prototypes for internal functions and utilities
void error(const char *fmt, ...);
int runClient(char *clientName, int numMessages, char **messages);
int runServer();
void serverReady(int signal);

bool serverIsReady = false;

// Prototypes for functions to be implemented by students
void server();
void client(char *clientName, int numMessages, char *messages[]);

void verror(const char *fmt, va_list argp)
{
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, argp);
    fprintf(stderr, "\n");
}

void error(const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    verror(fmt, argp);
    va_end(argp);
    exit(1);
}

int runServer(int port) {
    int forkPID = fork();
    if (forkPID < 0)
        error("ERROR forking server");
    else if (forkPID == 0) {
        server();
        exit(0);
    } else {
        printf("Main: Server(%d) launched...\n", forkPID);
    }
    return forkPID;
}

int runClient(char *clientName, int numMessages, char **messages) {
    fflush(stdout);
    printf("Launching client %s...\n", clientName);
    int forkPID = fork();
    if (forkPID < 0)

        error("ERROR forking client %s", clientName);
    else if (forkPID == 0) {
        client(clientName, numMessages, messages);
        exit(0);
    }
    return forkPID;
}

void serverReady(int signal) {
    serverIsReady = true;
}

#define NUM_CLIENTS 2
#define MAX_MESSAGES 5
#define MAX_CLIENT_NAME_LENGTH 10

struct client {
    char name[MAX_CLIENT_NAME_LENGTH];
    int num_messages;
    char *messages[MAX_MESSAGES];
    int PID;
};

// Modify these to implement different scenarios
struct client clients[] = {
        {"Uno", 3, {"Mensaje 1-1", "Mensaje 1-2", "Mensaje 1-3"}},
        {"Dos", 3, {"Mensaje 2-1", "Mensaje 2-2", "Mensaje 2-3"}}
};

int main() {
    signal(SIGUSR1, serverReady);
    int serverPID = runServer(getpid());
    while(!serverIsReady) {
        sleep(1);
    }
    printf("Main: Server(%d) signaled ready to receive messages\n", serverPID);

    for (int client = 0 ; client < NUM_CLIENTS ; client++) {
        clients[client].PID = runClient(clients[client].name, clients[client].num_messages,
                                        clients[client].messages);
    }

    for (int client = 0 ; client < NUM_CLIENTS ; client++) {
        int clientStatus;
        printf("Main: Waiting for client %s(%d) to complete...\n", clients[client].name, clients[client].PID);
        waitpid(clients[client].PID, &clientStatus, 0);
        printf("Main: Client %s(%d) terminated with status: %d\n",
               clients[client].name, clients[client].PID, clientStatus);
    }

    printf("Main: Killing server (%d)\n", serverPID);
    kill(serverPID, SIGINT);
    int statusServer;
    pid_t wait_result = waitpid(serverPID, &statusServer, 0);
    printf("Main: Server(%d) terminated with status: %d\n", serverPID, statusServer);
    return 0;
}



//*********************************************************************************
//**************************** EDIT FROM HERE *************************************
//#you can create the global variables and functions that you consider necessary***
//*********************************************************************************

#define PORT_NUMBER 40426

bool serverShutdown = false;

void shutdownServer(int signal) {
    pid_t wpid;

    //Indicate that the server has to shutdown
    serverShutdown = true;

    //Wait for the children to finish
    while ((wpid = waitpid(-1, NULL, 0)) > 0);

    //Exit
    exit(0);
}

void client(char *clientName, int numMessages, char *messages[]) {
    char buffer[256];
    int client_socket, n;
    struct sockaddr_in server_address;
    struct hostent *server;

    //For each message, write to the server and read the response
    //Accept connection and create a child process to read and write
    for(int i = 0; i < numMessages; i++) {
        //Open the socket
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(client_socket < 0)
            error("Error opening socket");

        server = gethostbyname("127.0.0.1");
        if (server == NULL) {
            fprintf(stderr,"ERROR, no such server\n");
            exit(0);
        }
        bzero((char *) &server_address, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(PORT_NUMBER);
        bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);

        //Connect to the server
        if((connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0))
            error("Failed to connect to the server.");

        strcpy(buffer, messages[i]);

        n = write(client_socket, buffer, 255);
        if(n < 0)
            error("ERROR on write");

        bzero(buffer,256);
        n = read(client_socket, buffer, 255);

        if(n < 0)
            error("ERROR on write");

        //Moodle Test Cases won't pass with this output format (???)
        //fprintf(stdout, "%s : %d : %s\n", clientName, getpid(), buffer); //expected output

        //Moodle Test Cases do pass with this output format
        fprintf(stdout, "Client %s(%d): Return message: %s\n", clientName, getpid(), buffer);
        fflush(stdout);
        sleep(1);
    }
    //Close socket
    close(client_socket);
}

void server() {

    int server_socket, message_socket, client_len, n;
    struct sockaddr_in server_address, client_address;
    char buffer[256];

    //Handle SIGINT so the server stops when the main process kills it
    signal(SIGINT, shutdownServer);

    //Open the socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
        error("ERROR opening socket");

    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT_NUMBER);

    //Bind the socket
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
        error("ERROR on binding");

    listen(server_socket, NUM_CLIENTS);

    //Signal server is ready
    kill(getppid(), SIGUSR1);

    //Accept connection
    client_len = sizeof(server_address);
    while(!serverShutdown) {
        message_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_len);

        //Create a child proccess to read and write
        int forkPID = fork();
        if(forkPID < 0)
            error("ERROR forking server child");
        else if(forkPID == 0){
            for(int i=0; i < MAX_MESSAGES;i++) {
                bzero(buffer, 256);

                n = read(message_socket, buffer, 255);
                if(n < 0)
                    error("ERROR reading from socket");

                //Moodle Test Cases won't pass with this output format (???)
                //fprintf(stdout, "Server : %d : %s\n", getpid(), buffer); //expected output

                //Moodle Test Cases do pass with this output format
                fprintf(stdout, "Server child(%d): got message: %s\n", getpid(), buffer);
                fflush(stdout);

                n = write(message_socket, buffer, 255);
                if(n < 0)
                    error("ERROR writing to socket");

                //Set up signal such that after 10 seconds of sleep, it exits
                signal(SIGALRM, exit);
                alarm(10);

                for(int j = 1; j <= 10; j++){
                    fprintf(stdout, "Child server(%d): sleeping %d...\n", getpid(), j);
                    sleep(1);
                }
            }
        }
        else close(message_socket); //Close socket used for message
    }
    //Close socket
    close(server_socket);
}