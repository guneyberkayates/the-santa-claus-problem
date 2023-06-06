#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_ELVES 3
#define NUM_REINDEER 9

sem_t santaSem, reindeerSem, elfTex, mutex;
int numElves = 0;
int numReindeerReturned = 0;

void *santa(void *arg) {
    while (1) {
        sem_wait(&santaSem);

        sem_wait(&mutex);
        if (numReindeerReturned == NUM_REINDEER) {
            printf("Santa Claus: Preparing sleigh for the reindeer!\n");
            // Harness reindeer to sleigh
            sleep(2);
            printf("Santa Claus: Reindeer are ready! Christmas is 
coming!\n");
            numReindeerReturned = 0; // Reset reindeer count
            sem_post(&reindeerSem); // Signal reindeer to deliver presents
            sem_post(&mutex);
            break;
        } else if (numElves == NUM_ELVES) {
            printf("Santa Claus: Helping the elves!\n");
            // Help the elves
            sleep(1);
            numElves = 0; // Reset elf count
            printf("Santa Claus: Elves are happy now!\n");
        }
        sem_post(&mutex);

        sem_post(&santaSem); // Release Santa Claus
    }

    pthread_exit(NULL);
}

void *reindeer(void *arg) {
    int id = *((int *)arg);
    sleep(id); // Reindeer return at different times

    sem_wait(&mutex);
    numReindeerReturned++;
    if (numReindeerReturned == NUM_REINDEER) {
        sem_post(&santaSem); // Signal Santa Claus
    }
    sem_post(&mutex);

    printf("Reindeer %d returned from vacation!\n", id);

    pthread_exit(NULL);
}

void *elf(void *arg) {
    int id = *((int *)arg);
    sleep(id); // Elves need help at different times

    sem_wait(&elfTex);

    sem_wait(&mutex);
    numElves++;
    if (numElves == NUM_ELVES) {
        sem_post(&santaSem); // Signal Santa Claus
    }
    sem_post(&mutex);

    printf("Elf %d needs help from Santa Claus!\n", id);
    // Get help from Santa Claus
    sleep(1);
    printf("Elf %d is happy now!\n", id);

    sem_wait(&mutex);
    numElves--;
    if (numElves == 0) {
        sem_post(&elfTex);
    }
    sem_post(&mutex);

    pthread_exit(NULL);
}

int main() {
    pthread_t santaThread, reindeerThreads[NUM_REINDEER], 
elfThreads[NUM_ELVES];

    sem_init(&santaSem, 0, 0);
    sem_init(&reindeerSem, 0, 0);
    sem_init(&elfTex, 0, 1);
    sem_init(&mutex, 0, 1);

    pthread_create(&santaThread, NULL, santa, NULL);

    int i;
    for (i = 0; i < NUM_REINDEER; i++) {
        int *reindeerId = malloc(sizeof(int));
        *reindeerId = i;
        pthread_create(&reindeerThreads[i], NULL, reindeer, (void 
*)reindeerId);
    }

    for (i = 0; i < NUM_ELVES; i++) {
        int *elfId = malloc(sizeof(int));
        *elfId = i;
        pthread_create(&elfThreads[i], NULL, elf, (void *)elfId);
    }

    pthread_join(santaThread, NULL);

    for (i = 0; i < NUM_REINDEER; i++) {
        pthread_join(reindeerThreads[i], NULL);
    }

    for (i = 0; i < NUM_ELVES; i++) {
        pthread_join(elfThreads[i], NULL);
    }

    sem_destroy(&santaSem);
    sem_destroy(&reindeerSem);
    sem_destroy(&elfTex);
    sem_destroy(&mutex);

    return 0;
}

