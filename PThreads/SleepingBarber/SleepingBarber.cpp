#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Definindo o número de cadeiras na sala de espera
#define NUM_CADEIRAS 5
#define NUM_CLIENTES 50

// Declarando mutex e variáveis de condição
pthread_mutex_t mutex;
pthread_cond_t cond_cliente;
pthread_cond_t cond_barbeiro;

int clientes_esperando = 0;  // Contador de clientes esperando

void* cliente(void* id) {
    int cliente_id = *(int*)id;
    free(id);

    pthread_mutex_lock(&mutex);
    if (clientes_esperando < NUM_CADEIRAS) {
        clientes_esperando++;
        printf("Cliente %d está esperando. Clientes na espera: %d\n", cliente_id, clientes_esperando);

        // Notifica o barbeiro de que há um cliente esperando
        pthread_cond_signal(&cond_cliente);

        // Espera até ser atendido pelo barbeiro
        pthread_cond_wait(&cond_barbeiro, &mutex);
        printf("Cliente %d está sendo atendido.\n", cliente_id);
    } else {
        printf("Cliente %d foi embora, sala de espera cheia.\n", cliente_id);
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* barbeiro(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (clientes_esperando == 0) {
            printf("Barbeiro está dormindo.\n");
            pthread_cond_wait(&cond_cliente, &mutex);  // Dorme até que um cliente chegue
        }

        // Atende o próximo cliente
        clientes_esperando--;
        printf("Barbeiro acordou e está atendendo um cliente. Clientes na espera: %d\n", clientes_esperando);

        // Notifica um cliente de que ele está sendo atendido
        pthread_cond_signal(&cond_barbeiro);
        pthread_mutex_unlock(&mutex);

        // Simula o tempo de atendimento
        sleep(2);
    }
}

int main() {
    pthread_t tid_barbeiro;
    pthread_t tid_cliente[NUM_CLIENTES];  // NUM_CLIENTES clientes para este exemplo

    // Inicializando mutex e variáveis de condição
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_cliente, NULL);
    pthread_cond_init(&cond_barbeiro, NULL);

    // Criando a thread do barbeiro
    pthread_create(&tid_barbeiro, NULL, barbeiro, NULL);

    // Criando as threads dos clientes
    for (int i = 0; i < NUM_CLIENTES; i++) {
        int* id = (int*)malloc(sizeof(int));  // Conversão explícita de void* para int*
        *id = i + 1;
        pthread_create(&tid_cliente[i], NULL, cliente, id);
        sleep(rand() % 3);  // Simula o intervalo variável entre a chegada dos clientes
    }

    // Esperando as threads dos clientes terminarem
    for (int i = 0; i < NUM_CLIENTES; i++) {
        pthread_join(tid_cliente[i], NULL);
    }

    // Encerrando a thread do barbeiro
    pthread_cancel(tid_barbeiro);

    // Destruindo mutex e variáveis de condição
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_cliente);
    pthread_cond_destroy(&cond_barbeiro);

    return 0;
}
