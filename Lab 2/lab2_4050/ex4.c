#include <stdio.h>
#include <stdlib.h>
#include <signal.h>


void SIGINT_handler();
void SIGQUIT_handler();
void SIGTERM_handler();
void SIGFPE_handler();
void SIGSEGV_handler();
void SIGILL_handler();

void setup_signal_handlers() {
    // Setup handlers here
    if (signal(SIGINT, SIGINT_handler) == SIG_ERR) {
        printf("SIGINT install error\n");
        exit(1);
    }

    if (signal(SIGINT, SIGINT_handler) == SIG_ERR) {
        printf("SIGINT install error\n");
        exit(1);
    }
    if (signal(SIGQUIT, SIGQUIT_handler) == SIG_ERR) {
        printf("SIGQUIT install error\n");
        exit(1);
    }
    if (signal(SIGTERM, SIGTERM_handler) == SIG_ERR) {
        printf("SIGTERM install error\n");
        exit(1);
    }
    if (signal(SIGFPE, SIGFPE_handler) == SIG_ERR) {
        printf("SIGFPE install error\n");
        exit(1);
    }
    if (signal(SIGSEGV, SIGSEGV_handler) == SIG_ERR) {
        printf("SIGSEGV install error\n");
        exit(1);
    }
    if (signal(SIGILL, SIGILL_handler) == SIG_ERR) {
        printf("SIGILL install error\n");
        exit(1);
    }
}

void SIGINT_handler() {
    printf("Received signal SIGINT!!!\n");
}
void SIGQUIT_handler(){
    printf("Received signal SIGQUIT!!!\n");
}
void SIGTERM_handler(){
    printf("Received signal SIGTERM!!!\n");
}
void SIGFPE_handler(){
    printf("Received signal SIGFPE!!!\n");
}
void SIGSEGV_handler(){
    printf("Received signal SIGSEGV!!!\n");
}
void SIGILL_handler(){
    printf("Received signal SIGILL!!!\n");
}