#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "util.h"

typedef struct threadpool{

    pthread_mutex_t lock;       // mutua esclusione nell'accesso al pool
    pthread_cond_t full;        // variabile di condizione per coda piena
    pthread_cond_t empty;       // variabile di condizione per coda vuota

    pthread_t * threads;        // array di thread
    int numthread;              // dimensione di threads;

    char **queue;               // coda interna per task pendenti
    int queue_max_size;         // dimensione massima della coda
    int queue_task;             // numero di task all'interno della coda
    int head, tail;             // riferimenti della coda

    int exiting;                // se = 1 e' iniziato il protocollo di uscita
} tpool;

/**
 * @brief crea un oggetto thread pool
 * @param numThreadInit e' il numero di thread iniziale del pool
 * @param pending_size e' la massima dimensione della coda 
 * @return restituisce un oggetto pool
*/
tpool * createThreadPool(int numThreadInit, int pending_size);

/** 
 * @brief aggiunge un task al pool
 * @param pool e' l'oggetto thread pool
 * @param str e' il nome del file
*/
void addTaskToThreadPool(tpool * pool, char * str);

/**
 * @brief termina i thread e dealloca le risorse allocate al pool
 * @param pool e' l'oggetto thread pool
 * @return 0 in caso di successo, -1 in caso di fallimento
*/
int destroyThreadPool(tpool * pool);

/**
 * @brief crea un nuovo thread e lo aggiunge al pool
 * @param pool e' l'oggetto thread pool
*/
void createNewThread(tpool *pool);
