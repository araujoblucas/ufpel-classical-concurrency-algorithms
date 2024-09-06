#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <unistd.h>  // Para a função sleep()

#define BUFFER_SIZE 5
#define NUM_ITEMS 10  // Número de itens a serem produzidos/consumidos
#define NUM_CONSUMIDORES 3  // Número de consumidores

int buffer[BUFFER_SIZE];
int in = 0, out = 0;  // Índices de entrada e saída do buffer
int count = 0;        // Contagem de itens no buffer

void produtor() {
    for (int i = 0; i < NUM_ITEMS; i++) {
        int item = rand() % 100;  // Item produzido
        int item_colocado = 0;

        // Espera até que o buffer tenha espaço
        while (!item_colocado) {
#pragma omp critical
            {
                if (count < BUFFER_SIZE) {
                    buffer[in] = item;
                    printf("Produtor colocou: %d (Posição %d)\n", item, in);
                    in = (in + 1) % BUFFER_SIZE;
                    count++;
                    item_colocado = 1;  // Marca que o item foi colocado
                }
            }
#pragma omp flush  // Garante que a variável count seja visível para todas as threads
        }

        sleep(rand() % 2 + 1);  // Simula o tempo de produção
    }
}

void consumidor(int id) {
    for (int i = 0; i < NUM_ITEMS / NUM_CONSUMIDORES; i++) {
        int item_pego = 0;
        int item;

        // Espera até que o buffer tenha itens
        while (!item_pego) {
#pragma omp critical
            {
                if (count > 0) {
                    item = buffer[out];
                    printf("Consumidor %d pegou: %d (Posição %d)\n", id, item, out);
                    out = (out + 1) % BUFFER_SIZE;
                    count--;
                    item_pego = 1;  // Marca que o item foi consumido
                }
            }
#pragma omp flush  // Garante que a variável count seja visível para todas as threads
        }

        sleep(rand() % 2 + 1);  // Simula o tempo de consumo
    }
}

int main() {
    // Define que vamos usar 3 threads (1 produtor e 2 consumidores)
    omp_set_num_threads(NUM_CONSUMIDORES + 1);

#pragma omp parallel
    {
        int id = omp_get_thread_num();
        if (id == 0) {
            // Thread 0 executa o produtor
            produtor();
        } else {
            // As demais threads executam os consumidores
            consumidor(id);
        }
    }

    return 0;
}
