#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 20

int buffer [BUFFER_SIZE];
int front = 0;
int rear = 0;
sem_t mutex;
sem_t full;
sem_t empty;

pthread_t tid;

void insert(int item){
    sem_wait(&empty); //If buffer is full, block producer
    sem_wait(&mutex); //Lock critical code

    buffer[front] = item;
    front = (front + 1) % BUFFER_SIZE;
    printf("front: %d, inserted: %d\n", front, item);
    sleep(1);

    sem_post(&mutex); //Release critical code
    sem_post(&full); //There is one space less in the buffer

}

int remove_item(){
    int item;
    sem_wait(&full); //If buffer is empty, block consumer
    sem_wait(&mutex); //Lock critical code

    item = buffer[rear];
    rear = (rear + 1) % BUFFER_SIZE;
    printf("rear: %d, removed: %d\n", rear, item);
    sleep(1);

    sem_post(&mutex); //Release critical code
    sem_post(&empty); //There is one space more available in the buffer


    return item;
}

void * producer(void * param){
    int item;
    while(1){
        item = rand() % BUFFER_SIZE;
        insert(item);
    }
}

void * consumer(void * param){
    int item;
    while(1){
        item = remove_item();
    }
}

int main(int argc, char * argv[]) {
    printf("Executing Producer/Consumer using Semaphores\n");
    int producers = atoi(argv[1]);
    int consumers = atoi(argv[2]);
    int i;

    sem_init(&mutex, 0, 1);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);

    for (i = 0; i < producers; i++)
        pthread_create(&tid, NULL, producer,NULL);

    for (i = 0; i < consumers; i++)
        pthread_create(&tid, NULL, consumer, NULL);

    pthread_join(tid,NULL);
    sem_destroy(&mutex);
    sem_destroy(&full);
    sem_destroy(&empty);
}
