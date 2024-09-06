#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>  // Para a função sleep()
#include <time.h>    // Para a função time()

// Ingredientes
#define TABACO 0
#define PAPEL 1
#define FOSFORO 2
#define RODADAS 5

bool mesa[3] = {false, false, false};  // Ingredientes na mesa
bool ingredientes_disponiveis = false; // Indica se Peter já colocou os ingredientes
int rodadas_restantes = RODADAS;       // Número de rodadas restantes
bool rodadas_acabaram = false;         // Flag para indicar quando as rodadas terminam

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex para sincronização

const char* ingredientes_nomes[] = {"Tabaco", "Papel", "Fósforo"};

void* fumante(void* arg) {
    int id = *(int*)arg;
    int ingrediente = id - 1;  // Assumindo que os IDs são 1, 2, 3

    bool continuar = true;
    while (continuar) {
        bool encontrou_ingredientes = false;

        pthread_mutex_lock(&mutex);
        // Verifica se os dois ingredientes que o fumante precisa estão na mesa
        if (ingredientes_disponiveis &&
            mesa[(ingrediente + 1) % 3] && mesa[(ingrediente + 2) % 3]) {
            printf("Fumante %d (que tem %s) pegou %s e %s e está fumando.\n",
                   id,
                   ingredientes_nomes[ingrediente],
                   ingredientes_nomes[(ingrediente + 1) % 3],
                   ingredientes_nomes[(ingrediente + 2) % 3]);

            // Limpa a mesa
            mesa[(ingrediente + 1) % 3] = false;
            mesa[(ingrediente + 2) % 3] = false;
            ingredientes_disponiveis = false;  // Ingredientes usados
            encontrou_ingredientes = true;
        }
        pthread_mutex_unlock(&mutex);

        if (encontrou_ingredientes) {
            sleep(rand() % 3 + 1);  // Simula o tempo de fumar
            printf("Fumante %d terminou de fumar.\n", id);
        } else {
            printf("Fumante %d (que tem %s) tentou pegar, mas os ingredientes corretos não estavam na mesa.\n",
                   id,
                   ingredientes_nomes[ingrediente]);
        }

        sleep(1);  // Fumante espera um pouco antes de verificar novamente

        pthread_mutex_lock(&mutex);
        if (rodadas_acabaram) continuar = false;  // Sai do loop quando as rodadas terminarem
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* peter(void* arg) {
    bool continuar = true;
    while (continuar) {
        pthread_mutex_lock(&mutex);
        if (rodadas_restantes <= 0) {
            rodadas_acabaram = true;  // Marca que as rodadas acabaram
            continuar = false;  // Sai do loop
        }

        if (!ingredientes_disponiveis && rodadas_restantes > 0) {
            // Peter escolhe dois ingredientes aleatórios
            int ingrediente1 = rand() % 3;
            int ingrediente2 = (ingrediente1 + 1 + rand() % 2) % 3;

            mesa[ingrediente1] = true;
            mesa[ingrediente2] = true;
            ingredientes_disponiveis = true;

            printf("Peter colocou %s e %s na mesa. (Rodada %d)\n",
                   ingredientes_nomes[ingrediente1],
                   ingredientes_nomes[ingrediente2],
                   RODADAS - rodadas_restantes + 1);

            rodadas_restantes--;  // Diminui o número de rodadas restantes
        }
        pthread_mutex_unlock(&mutex);

        sleep(1);  // Peter coloca ingredientes periodicamente
    }

    printf("Peter terminou de colocar ingredientes após 5 rodadas.\n");
    return NULL;
}

int main() {
    srand(time(NULL));  // Inicializa o gerador de números aleatórios

    pthread_t thread_peter;
    pthread_t fumantes[3];
    int ids[3] = {1, 2, 3};

    // Cria a thread de Peter
    pthread_create(&thread_peter, NULL, peter, NULL);

    // Cria as threads dos fumantes
    for (int i = 0; i < 3; i++) {
        pthread_create(&fumantes[i], NULL, fumante, &ids[i]);
    }

    // Espera a thread de Peter terminar
    pthread_join(thread_peter, NULL);

    // Espera as threads dos fumantes terminarem
    for (int i = 0; i < 3; i++) {
        pthread_join(fumantes[i], NULL);
    }

    printf("Simulação concluída após 5 rodadas.\n");

    return 0;
}
