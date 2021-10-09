#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

long long* result;

void* runner(void * param){

    int n = *((int*) param);

    for(int i = 0; i < n; i++){
        if(i == 0){
            *(result + 0) = 0;
        } else if (i == 1){
            *(result + 1) = 1;
        } else {
            result[i] = result[i-1] + result[i-2];
//            *(result + i) = *(result + (i-1)) + *(result + (i-2));
        }
    }

    pthread_exit(0);

}

int main(int argc, char *argv[]){

    int n = atoi(argv[1]);
    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);

    result = malloc(sizeof(long long) * n);

    pthread_create(&tid, &attr, runner, &n);

    pthread_join(tid, NULL);

    for(int i = 0; i < n; i++){
        printf(" %lld", result[i]);
    }
    printf("\n");
}