#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <stdbool.h>
#include <unistd.h> // Para a função sleep()

#define MAX_CADEIRAS 10

int num_cadeiras;
int num_clientes;
int num_cortes;
int cadeiras_ocupadas = 0;
int cortes_realizados = 0;
bool barbearia_aberta = true;
bool barbeiro_ativo = true; // Flag para controle do barbeiro

void *barbeiro_func(void *arg) {
    while (barbearia_aberta || cadeiras_ocupadas > 0) {
        bool need_to_sleep = false;

#pragma omp critical
        {
            if (cortes_realizados >= num_cortes && cadeiras_ocupadas == 0) {
                barbearia_aberta = false;
                barbeiro_ativo = false; // Finaliza o barbeiro
            }

            if (cadeiras_ocupadas > 0) {
                cadeiras_ocupadas--;
                printf("Barbeiro está cortando cabelo\n");
                sleep(rand() % 3 + 1);  // Simula o tempo de corte
                printf("Barbeiro terminou o corte\n");
                cortes_realizados++;
            } else {
                need_to_sleep = true;
            }
        }

        if (need_to_sleep) {
            printf("Barbeiro está aguardando clientes\n");
            sleep(1);  // Simula o tempo de espera
        }
    }

    printf("Barbeiro foi para casa\n");
    return NULL;
}

void *cliente_func(void *arg) {
    int id = *(int*)arg;

    while (true) {
        bool atendido = false;

#pragma omp critical
        {
            if (cadeiras_ocupadas < num_cadeiras && barbearia_aberta) {
                cadeiras_ocupadas++;
                printf("Cliente %d sentou na sala de espera\n", id);
                atendido = true;
            }
        }

        if (atendido) {
            printf("Cliente %d está tendo o cabelo cortado\n", id);
            sleep(rand() % 3 + 1);  // Simula o tempo de corte
            printf("Cliente %d terminou o corte\n", id);
            return NULL;
        } else {
            printf("Cliente %d foi embora (barbearia cheia ou fechada)\n", id);
            sleep(1);  // Espera um pouco antes de tentar novamente
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s <num_cadeiras> <num_clientes> <num_cortes>\n", argv[0]);
        return 1;
    }

    num_cadeiras = atoi(argv[1]);
    num_clientes = atoi(argv[2]);
    num_cortes = atoi(argv[3]);

    if (num_cadeiras > MAX_CADEIRAS) {
        printf("Número máximo de cadeiras é %d\n", MAX_CADEIRAS);
        return 1;
    }

    omp_set_num_threads(2);  // Define o número de threads OpenMP

#pragma omp parallel sections
    {
#pragma omp section
        {
            barbeiro_func(NULL);
        }

#pragma omp section
        {
            int ids[num_clientes];
#pragma omp parallel for
            for (int i = 0; i < num_clientes; i++) {
                ids[i] = i;
                cliente_func(&ids[i]);
                sleep(rand() % 2);  // Intervalo aleatório entre chegadas de clientes
            }
        }
    }

    // Espera o barbeiro finalizar o trabalho
    while (barbeiro_ativo) {
        sleep(1);
    }

    printf("Total de cortes realizados: %d\n", cortes_realizados);

    return 0;
}

