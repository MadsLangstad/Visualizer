#include "dataGen.h"
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static int shared_data = 0;
static pthread_t generator_thread;
static sem_t data_sem;
static volatile int keep_running = 1;


/* Thread function that generates data */
static void* generator_func(void* arg) {
    while (keep_running) {
        /* Simulate data generation (e.g., incrementing the shared_data) */
        int new_data = rand() % 1000000000; /* Generate random data */
        /* Update shared_data atomically */
        sem_wait(&data_sem);
        shared_data = new_data;
        sem_post(&data_sem);
        
        /* Sleep for a short duration to simulate work */
        usleep(100000); /* 100 milliseconds */
    }
    return NULL;
}


int data_get_init() {
    /* Initialize semaphore */
    if (sem_init(&data_sem, 0, 1) != 0) {
        perror("Failed to initialize semaphore");
        return -1;
    }
    
    /* Seed the random number generator */
    srand((unsigned int)time(NULL));
    
    /* Create the generator thread */
    if (pthread_create(&generator_thread, NULL, generator_func, NULL) != 0) {
        perror("Failed to create generator thread");
        sem_destroy(&data_sem);
        return -1;
    }
    
    return 0;
}

void data_gen_cleanup() {
    /* Signal the thread to stop */
    keep_running = 0;
    
    /* Wait for thread to finish */
    pthread_join(generator_thread, NULL);
    
    /* Destroy sem */
    sem_destroy(&data_sem);
}


int data_gen_get_data() {
    int data;
    
    /* Read shared_data safely */
    sem_wait(&data_sem);
    data = shared_data;
    sem_post(&data_sem);
    
    return data;
}