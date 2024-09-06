#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>  // Para a função sleep()

#define BUFFER_SIZE 5
#define NUM_ITEMS 10  // Número de itens a serem produzidos/consumidos
#define NUM_CONSUMIDORES 3  // Número de consumidores

int buffer[BUFFER_SIZE];
int in = 0, out = 0;  // Índices de entrada e saída do buffer

// Semáforos e mutex para sincronização
sem_t empty;  // Controla quantos espaços vazios no buffer
sem_t full;   // Controla quantos itens no buffer
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* produtor(void* arg) {
    for (int i = 0; i < NUM_ITEMS; i++) {
        int item = rand() % 100;  // Item produzido

        // Espera se o buffer estiver cheio
        sem_wait(&empty);  // Diminui o contador de espaços vazios

        // Seção crítica para colocar item no buffer
        pthread_mutex_lock(&mutex);
        buffer[in] = item;
        printf("Produtor colocou: %d (Posição %d)\n", item, in);
        in = (in + 1) % BUFFER_SIZE;
        pthread_mutex_unlock(&mutex);

        // Sinaliza que há um item no buffer
        sem_post(&full);  // Aumenta o contador de itens disponíveis

        sleep(rand() % 2 + 1);  // Simula tempo de produção
    }
    return NULL;
}

void* consumidor(void* arg) {
    int id = *(int*)arg;  // O ID do consumidor é passado como argumento

    for (int i = 0; i < NUM_ITEMS / NUM_CONSUMIDORES; i++) {
        // Espera se o buffer estiver vazio
        sem_wait(&full);  // Diminui o contador de itens no buffer

        // Seção crítica para retirar item do buffer
        pthread_mutex_lock(&mutex);
        int item = buffer[out];
        printf("Consumidor %d pegou: %d (Posição %d)\n", id, item, out);
        out = (out + 1) % BUFFER_SIZE;
        pthread_mutex_unlock(&mutex);

        // Sinaliza que há espaço no buffer
        sem_post(&empty);  // Aumenta o contador de espaços disponíveis

        sleep(rand() % 2 + 1);  // Simula tempo de consumo
    }
    return NULL;
}

int main() {
    pthread_t prod_thread, cons_threads[NUM_CONSUMIDORES];
    int cons_ids[NUM_CONSUMIDORES];

    // Inicializa os semáforos
    sem_init(&empty, 0, BUFFER_SIZE);  // Semáforo para itens vazios
    sem_init(&full, 0, 0);             // Semáforo para itens cheios

    // Cria a thread do produtor
    pthread_create(&prod_thread, NULL, produtor, NULL);

    // Cria as threads dos consumidores
    for (int i = 0; i < NUM_CONSUMIDORES; i++) {
        cons_ids[i] = i + 1;  // Define o ID para o consumidor (começando em 1)
        pthread_create(&cons_threads[i], NULL, consumidor, &cons_ids[i]);
    }

    // Espera a thread do produtor terminar
    pthread_join(prod_thread, NULL);

    // Espera as threads dos consumidores terminarem
    for (int i = 0; i < NUM_CONSUMIDORES; i++) {
        pthread_join(cons_threads[i], NULL);
    }

    // Destroi os semáforos
    sem_destroy(&empty);
    sem_destroy(&full);

    return 0;
}
