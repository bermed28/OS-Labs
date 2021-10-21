#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 5
int buffer [BUFFER_SIZE];
pthread_t tid;
pthread_cond_t notEmpty;
pthread_cond_t notFull;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int front = 0;
int rear = 0;

void insert(int item) {

    while((rear - front + BUFFER_SIZE) % BUFFER_SIZE == BUFFER_SIZE) pthread_cond_wait(&notFull, &mutex); //Block Producer, release mutex since buffer is full

    //Insert into circular buffer and move front forward to point to next available spot
    buffer[front] = item;
    front = (front + 1) % BUFFER_SIZE;
    printf("front: %d, inserted: %d\n", front, item);
    sleep(1);

    pthread_cond_signal(&notEmpty); //Wake consumer, buffer has at least one element consumer can consume

}

int remove_item() {

    int item;

    while((rear - front + BUFFER_SIZE) % BUFFER_SIZE  == 0) pthread_cond_wait(&notEmpty, &mutex); //Block Consumer, release mutex since buffer is empty

    //Remove from circular buffer and move rear ones spot forward to point to next item to consume
    item = buffer[rear];
    rear = (rear + 1) % BUFFER_SIZE;
    printf("rear: %d, removed: %d\n", rear, item);
    sleep(1);

    pthread_cond_signal(&notFull); //Wake producer, buffer has at least one space open producer can insert

    return item;
}

void * producer(void * param) {
    int item;
    while(1){
        item = rand() % BUFFER_SIZE;
        insert(item);
    }
}

void * consumer(void * param) {
    int item;
    while(1){
        item = remove_item();

    }
}

int main(int argc, char * argv[]) {
    printf("Executing Producer/Consumer using Monitors\n");
    int producers = atoi(argv[1]);
    int consumers = atoi(argv[2]);
    int i;

    pthread_cond_init(&notEmpty, NULL);
    pthread_cond_init(&notFull, NULL);

    for (i = 0; i < producers; i++)
        pthread_create(&tid, NULL, producer, NULL);
    for (i = 0; i < consumers; i++)
        pthread_create(&tid, NULL, consumer,  NULL);

    pthread_join(tid,NULL);
    pthread_cond_destroy(&notFull);
    pthread_cond_destroy(&notEmpty);


}