#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_FILOSOFOS 10

pthread_mutex_t garfos[MAX_FILOSOFOS];

void* jantar(void* arg) {
    int id = *(int*)arg;
    int num_filosofos = MAX_FILOSOFOS;
    int limite_refeicoes = 3; // Exemplo de limite de refeições
    int esquerda = id;
    int direita = (id + 1) % num_filosofos;
    int refeicoes = 0;

    while (refeicoes < limite_refeicoes) {
        printf("Filósofo %d está pensando...\n", id);
        usleep(rand() % 1000000); // Pensando

        if (pthread_mutex_trylock(&garfos[esquerda]) == 0) {
            printf("Filósofo %d pegou o garfo %d.\n", id, esquerda);

            if (pthread_mutex_trylock(&garfos[direita]) == 0) {
                printf("Filósofo %d pegou o garfo %d.\n", id, direita);
                printf("Filósofo %d está comendo a refeição %d...\n", id, refeicoes + 1);
                usleep(rand() % 1000000); // Comendo

                pthread_mutex_unlock(&garfos[direita]);
                printf("Filósofo %d devolveu o garfo %d.\n", id, direita);

                refeicoes++;
            }

            pthread_mutex_unlock(&garfos[esquerda]);
            printf("Filósofo %d devolveu o garfo %d.\n", id, esquerda);
        }

        usleep(rand() % 1000000); // Espera antes de tentar novamente
    }

    printf("Filósofo %d terminou suas refeições.\n", id);
    free(arg);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <numero_de_filosofos> <limite_de_refeicoes>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_filosofos = atoi(argv[1]);
    int limite_refeicoes = atoi(argv[2]);

    if (num_filosofos <= 0 || limite_refeicoes <= 0 || num_filosofos > MAX_FILOSOFOS) {
        fprintf(stderr, "Número de filósofos deve ser entre 1 e %d e o limite de refeições deve ser maior que 0.\n", MAX_FILOSOFOS);
        return EXIT_FAILURE;
    }

    pthread_t threads[num_filosofos];

    for (int i = 0; i < num_filosofos; i++) {
        pthread_mutex_init(&garfos[i], NULL);
    }

    for (int i = 0; i < num_filosofos; i++) {
        int* id = (int*)malloc(sizeof(int));  // Conversão explícita de void* para int*
        *id = i;
        pthread_create(&threads[i], NULL, jantar, id);
    }

    for (int i = 0; i < num_filosofos; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < num_filosofos; i++) {
        pthread_mutex_destroy(&garfos[i]);
    }

    return EXIT_SUCCESS;
}
