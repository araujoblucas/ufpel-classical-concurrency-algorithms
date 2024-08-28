#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_FILOSOFOS 10
#define OCUPADO       0
#define LIVRE         !OCUPADO

void jantar(int id, int num_filosofos, int *garfos, int limite_refeicoes) {
    int esquerda = id;
    int direita = (id + 1) % num_filosofos;
    int refeicoes = 0;
    bool pegou_esquerda;
    bool pegou_direita;

    while (refeicoes < limite_refeicoes) {
        pegou_esquerda = false;
        pegou_direita = false;

        printf("Filósofo %d está pensando...\n", id);
        usleep(rand() % 1000000); // Pensando

        // Tentar pegar o garfo da esquerda
        #pragma omp critical
        {
           if (garfos[esquerda] == LIVRE) {
               garfos[esquerda] = OCUPADO; // Garfo à esquerda é pego
               pegou_esquerda = true;
               printf("Filósofo %d pegou o garfo %d.\n", id, esquerda);
           }
        }

	if( pegou_esquerda == false ) {
	    usleep(1000000);
	    continue;
	} else { // Se conseguiu pegar o garfo da esquerda, tentar pegar o da direita
	    for(int tentativas = 5 ; tentativas ; --tentativas ) {
                #pragma omp critical
                {
		    if (garfos[direita] == LIVRE) {
                        garfos[direita] = OCUPADO; // Garfo à direita é pego
                        pegou_direita = true;
                        printf("Filósofo %d pegou o garfo %d.\n", id, direita);
                    }
                }
		if (pegou_direita == false) usleep(1000000);
            }

            if (pegou_direita) { // Ambos os garfos foram pegos
                printf("Filósofo %d está comendo a refeição %d...\n", id, refeicoes + 1);
                usleep(rand() % 1000000); // Comendo
                #pragma omp critical
                {
                    garfos[esquerda] = LIVRE; // Garfo à esquerda é devolvido
                    garfos[direita]  = LIVRE; // Garfo à direita é devolvido
                    printf("Filósofo %d devolveu os garfos %d e %d.\n", id, esquerda, direita);
                }
                refeicoes++;
            } else {
                    #pragma omp critical
                    {
                        garfos[esquerda] = LIVRE; // Garfo à esquerda é devolvido
                        printf("Filósofo %d liberou o garfo %d após falhar em pegar o garfo %d.\n", id, esquerda, direita);
                    }
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

