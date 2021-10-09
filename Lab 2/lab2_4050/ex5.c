#include <stdio.h>

void generate_SIGFPE() {
    int a = 3, b = 0;

    a = a / b;

}

void generate_SIGSEGV() {
    char *p = 0; // nil pointer
    fputc(*p, stdout); // try to use the pointer
}