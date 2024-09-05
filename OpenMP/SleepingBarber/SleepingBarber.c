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

void barbeiro_func() {
    while (barbearia_aberta || cadeiras_ocupadas > 0) {
        bool need_to_sleep = false;

#pragma omp critical
        {
            if (cortes_realizados >= num_cortes && cadeiras_ocupadas == 0) {
                barbearia_aberta = false; // Fecha a barbearia se todos os cortes forem feitos
            }

            if (cadeiras_ocupadas > 0) {
                cadeiras_ocupadas--;
                printf("Barbeiro está cortando cabelo\n");
                sleep(rand() % 3 + 1);  // Simula o tempo de corte
                printf("Barbeiro terminou o corte\n");
                cortes_realizados++;
            } else {
                need_to_sleep = true;  // Barbeiro deve esperar se não houver clientes
            }
        }

        if (need_to_sleep) {
            printf("Barbeiro está aguardando clientes\n");
            sleep(1);  // Simula o tempo de espera
        }
    }

    printf("Barbeiro foi para casa\n");
}

void cliente_func(int id) {
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
            return;  // O cliente foi atendido e sai da função
        } else {
            printf("Cliente %d foi embora (barbearia cheia ou fechada)\n", id);
            sleep(1);  // Espera antes de tentar novamente
        }

        if (!barbearia_aberta) {
            break;  // Sai do loop se a barbearia estiver fechada
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

    // Iniciamos as threads usando OpenMP
#pragma omp parallel sections
    {
        // Seção do barbeiro
#pragma omp section
        {
            barbeiro_func();
        }

        // Seção dos clientes
#pragma omp section
        {
#pragma omp parallel for
            for (int i = 0; i < num_clientes; i++) {
                cliente_func(i + 1);
                sleep(rand() % 2);  // Intervalo aleatório entre chegadas de clientes
            }
        }
    }

    printf("Total de cortes realizados: %d\n", cortes_realizados);

    return 0;
}
