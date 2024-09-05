#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <unistd.h> // Necessário para usar sleep

// Variáveis globais para controlar leitores e escritores
int active_readers = 0;      // Número de leitores ativos
int active_writers = 0;      // Número de escritores ativos
int waiting_writers = 0;     // Número de escritores esperando
int waiting_readers = 0;     // Número de leitores esperando

void reader(int id, int num_operations) {
    for (int i = 0; i < num_operations; i++) {
        sleep(rand() % 3);

        // Espera ativa para garantir que um escritor ativo não esteja presente
        #pragma omp atomic
        {
            waiting_readers++;
        }

        int can_read = 0;  // Variável de controle para determinar quando o leitor pode começar

        while (!can_read) {
            #pragma omp critical
            {
                if (active_writers == 0 && waiting_writers == 0) {
                    active_readers++;
                    waiting_readers--;
                    printf("Reader %d starts reading.\n", id);
                    can_read = 1;  // Atualiza a variável de controle para sair do loop
                }
            }
            #pragma omp flush // Garante que a leitura da variável é atualizada
        }

        // Seção crítica - leitura
        sleep(2); // Simula tempo de leitura

        #pragma omp critical
        {
            active_readers--;
            printf("Reader %d finished reading.\n", id);
        }

        sleep(2); // Simula tempo entre leituras
    }

    printf("Reader %d has completed all operations.\n", id);
}

void writer(int id, int num_operations) {
    for (int i = 0; i < num_operations; i++) {
        sleep(rand() % 3);

        #pragma omp atomic
        {
            waiting_writers++;
        }

        int can_write = 0;  // Variável de controle para determinar quando o escritor pode começar

        // Espera ativa para garantir que não há leitores ou escritores ativos
        while (!can_write) {
            #pragma omp critical
            {
                if (active_readers == 0 && active_writers == 0) {
                    active_writers++;
                    waiting_writers--;
                    printf("Writer %d starts writing.\n", id);
                    can_write = 1;  // Atualiza a variável de controle para sair do loop
                }
            }
        #pragma omp flush // Garante que a leitura da variável é atualizada
        }

        // Seção crítica - escrita
        sleep(2); // Simula tempo de escrita

        #pragma omp critical
        {
            active_writers--;
            printf("Writer %d finished writing.\n", id);
        }

        sleep(2); // Simula tempo entre escritas
    }

    printf("Writer %d has completed all operations.\n", id);
}

int main() {
    int num_readers, num_writers;
    int reader_operations, writer_operations;

    printf("Enter number of readers: ");
    scanf("%d", &num_readers);

    printf("Enter number of writers: ");
    scanf("%d", &num_writers);

    printf("Enter number of operations each reader will perform: ");
    scanf("%d", &reader_operations);

    printf("Enter number of operations each writer will perform: ");
    scanf("%d", &writer_operations);

    #pragma omp parallel num_threads(num_readers + num_writers)
    {
        int id = omp_get_thread_num();
        if (id < num_readers) {
            reader(id + 1, reader_operations);
        } else {
            writer(id - num_readers + 1, writer_operations);
        }
    }

    return 0;
}
