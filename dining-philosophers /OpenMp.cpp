#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5

omp_lock_t forks[NUM_PHILOSOPHERS];

void think(int philosopher) {
    printf("Philosopher %d is thinking...\n", philosopher);
    usleep(rand() % 1000); // Simulate thinking
}

void eat(int philosopher) {
    printf("Philosopher %d is eating...\n", philosopher);
    usleep(rand() % 1000); // Simulate eating
}

void philosopher(int philosopher) {
    int left_fork = philosopher;
    int right_fork = (philosopher + 1) % NUM_PHILOSOPHERS;

    while (1) {
        think(philosopher);

        // Lock the forks
        if (philosopher % 2 == 0) {
            omp_set_lock(&forks[left_fork]);
            omp_set_lock(&forks[right_fork]);
        } else {
            omp_set_lock(&forks[right_fork]);
            omp_set_lock(&forks[left_fork]);
        }

        eat(philosopher);

        // Unlock the forks
        omp_unset_lock(&forks[left_fork]);
        omp_unset_lock(&forks[right_fork]);
    }
}

int main() {
    omp_set_num_threads(NUM_PHILOSOPHERS);

    // Initialize the locks
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        omp_init_lock(&forks[i]);
    }

    // Create philosopher threads
    #pragma omp parallel
    {
        int philosopherIdentifier = omp_get_thread_num();
        philosopher(philosopherIdentifier);
    }

    // Destroy the locks
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        omp_destroy_lock(&forks[i]);
    }

    return 0;
}
