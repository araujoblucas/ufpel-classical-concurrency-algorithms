#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <unistd.h>

#define MAX_FILOSOFOS 10
#define LIVRE -1

void jantar(int id, int num_filosofos, int *garfos, int limite_refeicoes) {
    int esquerda = id;
    int direita = (id + 1) % num_filosofos;
    int refeicoes = 0;

    while (refeicoes < limite_refeicoes) {
        printf("Filósofo %d está pensando...\n", id);
        usleep(rand() % 1000000); // Pensando

        int tentativas = 5;

        // Tentar pegar o garfo da esquerda
        while (tentativas > 0) {
            int situacao = LIVRE;
            #pragma omp atomic compare
            if (garfos[esquerda] == LIVRE) {
                garfos[esquerda] = id;
            }
	    #pragma omp atomic read
	    situacao = garfos[esquerda];
	    if( situacao == id ) break;
            --tentativas;
            usleep(100000); // Espera antes de tentar novamente
        }

        // Se conseguiu pegar o garfo da esquerda, tentar pegar o garfo da direita
        if (garfos[esquerda] == id) {
            tentativas = 5;
            while (tentativas > 0) {
                int situacao = LIVRE;
                #pragma omp atomic compare
                if (garfos[direita] == LIVRE) {
                    garfos[direita] = id;
                }
	        #pragma omp atomic read
	        situacao = garfos[esquerda];
	        if( situacao == id ) break;
                --tentativas;
                usleep(100000); // Espera antes de tentar novamente
            }

            // Se conseguiu pegar ambos os garfos, comer; caso contrário, liberar o garfo da esquerda
            if (garfos[direita] == id) { // Verifica se pegou o garfo da direita
                printf("Filósofo %d está comendo a refeição %d...\n", id, refeicoes + 1);
                usleep(rand() % 1000000); // Comendo

                // Devolver os garfos
                #pragma omp atomic write
                garfos[esquerda] = LIVRE;

                #pragma omp atomic write
                garfos[direita] = LIVRE;

                printf("Filósofo %d devolveu os garfos %d e %d.\n", id, esquerda, direita);

                refeicoes++;
            } else {
                // Não conseguiu pegar o garfo da direita, liberar o garfo da esquerda
                #pragma omp atomic write
                garfos[esquerda] = LIVRE;
            }
        }
    }

    printf("Filósofo %d terminou suas refeições.\n", id);
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

    int garfos[num_filosofos];
    for (int i = 0; i < num_filosofos; i++) {
        garfos[i] = LIVRE; // Garfo disponível
    }

    #pragma omp parallel num_threads(num_filosofos)
    {
        int id = omp_get_thread_num();
        jantar(id, num_filosofos, garfos, limite_refeicoes);
    }

    return EXIT_SUCCESS;
}

