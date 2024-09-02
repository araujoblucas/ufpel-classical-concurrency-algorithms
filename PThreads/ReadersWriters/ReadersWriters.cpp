#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // Necessário para usar sleep

// Variáveis globais para controlar leitores e escritores
pthread_mutex_t mutex;       // Mutex para proteger a seção crítica
pthread_cond_t cond_readers; // Condição para leitores
pthread_cond_t cond_writers; // Condição para escritores
int active_readers = 0;      // Número de leitores ativos
int active_writers = 0;      // Número de escritores ativos
int waiting_writers = 0;     // Número de escritores esperando
int waiting_readers = 0;     // Número de leitores esperando

typedef struct {
    int id;
    int num_operations;
} ReaderWriterArgs;

void *reader(void *arg) {
    ReaderWriterArgs *args = (ReaderWriterArgs *)arg;
    int id = args->id;
    int num_operations = args->num_operations;

    for (int i = 0; i < num_operations; i++) {

        sleep(rand() % 3);
        pthread_mutex_lock(&mutex);
        waiting_readers++;
        printf("Reader %d is waiting in the queue.\n", id);
        while (active_writers > 0 || (waiting_writers > 0 && waiting_readers > 1)) {
            pthread_cond_wait(&cond_readers, &mutex);
        }
        waiting_readers--;
        active_readers++;
        printf("Reader %d starts reading.\n", id);
        pthread_mutex_unlock(&mutex);

        // Seção crítica - leitura
        sleep(2); // Simula tempo de leitura

        pthread_mutex_lock(&mutex);
        active_readers--;
        printf("Reader %d finished reading.\n", id);
        if (active_readers == 0 && waiting_writers > 0) {
            pthread_cond_signal(&cond_writers);
        } else if (waiting_readers > 0) {
            pthread_cond_broadcast(&cond_readers);
        }
        pthread_mutex_unlock(&mutex);

        sleep(2); // Simula tempo entre leituras
    }

    printf("Reader %d has completed all operations.\n", id);
    return NULL;
}

void *writer(void *arg) {
    ReaderWriterArgs *args = (ReaderWriterArgs *)arg;
    int id = args->id;
    int num_operations = args->num_operations;

    for (int i = 0; i < num_operations; i++) {
        pthread_mutex_lock(&mutex);
        waiting_writers++;
        printf("Writer %d is waiting in the queue.\n", id);
        while (active_readers > 0 || active_writers > 0) {
            pthread_cond_wait(&cond_writers, &mutex);
        }
        waiting_writers--;
        active_writers++;
        printf("Writer %d starts writing.\n", id);
        pthread_mutex_unlock(&mutex);

        // Seção crítica - escrita
        sleep(2); // Simula tempo de escrita

        pthread_mutex_lock(&mutex);
        active_writers--;
        printf("Writer %d finished writing.\n", id);
        if (waiting_writers > 0) {
            pthread_cond_signal(&cond_writers);
        } else {
            pthread_cond_broadcast(&cond_readers);
        }
        pthread_mutex_unlock(&mutex);

        sleep(2); // Simula tempo entre escritas
    }

    printf("Writer %d has completed all operations.\n", id);
    return NULL;
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

    pthread_t readers[num_readers];
    pthread_t writers[num_writers];
    ReaderWriterArgs reader_args[num_readers];
    ReaderWriterArgs writer_args[num_writers];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_readers, NULL);
    pthread_cond_init(&cond_writers, NULL);

    for (int i = 0; i < num_readers; i++) {
        reader_args[i].id = i + 1;
        reader_args[i].num_operations = reader_operations;
        pthread_create(&readers[i], NULL, reader, &reader_args[i]);
    }

    for (int i = 0; i < num_writers; i++) {
        writer_args[i].id = i + 1;
        writer_args[i].num_operations = writer_operations;
        pthread_create(&writers[i], NULL, writer, &writer_args[i]);
    }

    for (int i = 0; i < num_readers; i++) {
        pthread_join(readers[i], NULL);
    }

    for (int i = 0; i < num_writers; i++) {
        pthread_join(writers[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_readers);
    pthread_cond_destroy(&cond_writers);

    return 0;
}
