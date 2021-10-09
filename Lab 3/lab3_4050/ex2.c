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
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
// Prototypes for internal functions and utilities
void error(const char *fmt, ...);
int runClient(char *clientName, int numMessages, char **messages);
int runServer();
void serverReady(int signal);

pthread_t tidServer[2];

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
        pthread_join(tidServer[client],NULL);
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
//#you can create global variables and functions that you consider necessary***
//*********************************************************************************

#define PORT_NUMBER 18378

bool serverShutdown = false;


void shutdownServer(int signal) {
    //Indicate that the server has to shut down
    serverShutdown = true;
    //Wait for the children to finish
    while(wait(NULL) > 0);
    //Exit
    exit(0);
}


void client(char *clientName, int numMessages, char *messages[]) {
    char buffer[256];
    int client_socket, n;
    struct sockaddr_in server_address;
    struct hostent *host;

    //Open the socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0) {
        error("Error opening socket");
    }
    host = gethostbyname("127.0.0.1");
    if(host == NULL) {
        error("ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_NUMBER);
    bcopy((char *)host->h_addr, (char *)&server_address.sin_addr.s_addr, host->h_length);

    //Connect to the server
    if((connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0))
        error("Failed to connect to the server.");


    for(int i = 0; i < numMessages; i++) {

        //For each message, write to the server and read the response
        strcpy(buffer, messages[i]);
        n = write(client_socket, buffer, 255);
        if(n < 0)
            error("ERROR on write (client)");

        bzero(buffer,256);
        n = read(client_socket, buffer, 255);
        if(n < 0)
            error("ERROR on read (client)");

        fprintf(stdout, "Client %s(%d): Return message: %s\n", clientName, getpid(), buffer);
        fflush(stdout);
        sleep(1);
    }

    //Close socket
    close(client_socket);
}

void *pthreadServer(void * arguments) {
    char buffer[256];
    int i = 1;
    int n;
    int socket = *((int *) arguments);
    while (true) {

        bzero(buffer, 256);
        n = read(socket, buffer, 255);
        if (n < 0) error("ERROR reading from socket\n");

        while (n == 0 && i < 3) { // While we have an empty buffer, the server is sleeping
            printf("Server child(%d): sleeping %d...\n", getpid(), i);
            sleep(1);
            i++;
        }

        if (i == 3)
            pthread_exit(0);

        printf("Server child(%d): got message: %s\n", getpid(), buffer); //expected output
        fflush(stdout);

        n = write(socket, buffer, strlen(buffer));
        if (n < 0)
            error("ERROR writing to socket\n");
    }
}

void server() {
    int server_socket, temp_socket, client_length;
    struct sockaddr_in server_address, client_address;

    //Handle SIGINT so the server stops when the main process kills it
    signal(SIGINT, shutdownServer);

    //Open the socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if(server_socket < 0)
        error("ERROR opening socket");

    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT_NUMBER);

    //Bind the socket
    if(bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
        error("ERROR on binding");

    listen(server_socket, MAX_MESSAGES);

    //Signal server is ready
    kill(getppid(), SIGUSR1);

    //Accept connection and create a PTHREAD
    client_length = sizeof(server_address);
    int i = 0;

    while(!serverShutdown) {
        temp_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_length);
        if(temp_socket < 0) error("ERROR on accept");
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tidServer[i], &attr, pthreadServer, (void*) &temp_socket);
        i++;
        sleep(1);
    }
    //Close socket
    close(server_socket);
}