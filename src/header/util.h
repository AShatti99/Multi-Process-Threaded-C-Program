#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 2001112L
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

// ------------------------------ VALORI DI DEFAULT ------------------------------------

// lunghezza massima del nome del file in input
#define MAXFILENAME 255
#define SOCKNAME "./farm2.sck"


// ------------------------------ MSG DI ERRORE ----------------------------------------

// messaggio da restituire in caso in cui i parametri presi in ingresso non sono corretti
#define USAGE(arg)                                                                                            \
    fprintf(stderr, "\nusage: %s [-n <nthread>] [-q <qlen>] [-d <directory-name>] [-t <delay>] file [file]    \
    \ndefault value: -n 4 -q 8 -t 0\n\n", arg);


// ------------------------------ MACRO PER SINCRONIZZAZIONE ---------------------------

#define LOCK(l)                                     \
    if(pthread_mutex_lock(l) != 0) {                \
        fprintf(stderr, "fatal error lock\n");      \
        exit(EXIT_FAILURE);                         \
    }

#define UNLOCK(l)                                   \
    if(pthread_mutex_unlock(l) != 0){               \
        fprintf(stderr, "fatal error unlock\n");    \
        exit(EXIT_FAILURE);                         \
    }

#define WAIT(c, l)                                  \
    if(pthread_cond_wait(c, l) != 0){               \
        fprintf(stderr, "fatal error wait\n");      \
        exit(EXIT_FAILURE);                         \
    }

#define SIGNAL(c)                                   \
    if(pthread_cond_signal(c) != 0){                \
        fprintf(stderr, "fatal error signal\n");    \
        exit(EXIT_FAILURE);                         \
    }

#define BROADCAST(c)                                \
    if(pthread_cond_broadcast(c) != 0){             \
        fprintf(stderr, "fatal error broadcast\n"); \
        exit(EXIT_FAILURE);                         \
    }

// ---------------------------- FUNZIONI DI SUPPORTO -----------------------------------

/** @brief controlla se la stringa passata come prima argomento e' un numero e se sì > 0
 *  @param s e' la stringa da controllare
 *  @return stringa convertita in numero in caso di successo, -1 in caso di errore
*/
int check_number(const char* s);

/**
 * @brief verifica se arg corrisponde ad un file regolare o ad una directory
 * @param arg e' il percorso da controllare
 * @return -1 in caso di errore, 1 in caso di file regolare e 2 in caso di directory
*/
int check_file(const char * arg);

/**
 * @brief sospende l'esecuzione per un determinato numero di millisecondi
 * @param ms e' il numero di millisecondi per cui sospendere l'esecuzione
*/
int millisecondSleep(long ms);

/**
 * @brief legge esattamente size byte dal descrittore di file fd
 * @param fd e' il descrittore di file da cui leggere
 * @param buf e' il buffer da cui memorizzare i dati letti
 * @param size e' il numero di byte da leggere
 * @return size in caso di successo, 0 se durante la lettura leggo EOF, -1 in caso di errore
*/
int readn(long fd, void *buf, size_t size);

/**
 * @brief scrive esattamente size byte sul descrittore di file fd
 * @param fd e' il descrittore di file su cui scrivere
 * @param buf e' il buffer contenente i dati da scrivere
 * @param size e' il numero di byte da scrivere
 * @return 1 in caso di successo, 0 se la write ritorna 0, -1 in caso di errore
*/
int writen(long fd, void *buf, size_t size);

// ----------------------------- PER DEALLOCARE RISORSE ---------------------------------

/**
 * @brief dealloca il mutex se e' stato inizializzato
 * @param mtx e' il puntatore al mutex da deallocare
*/
void cleanupLock(pthread_mutex_t *mtx);

/**
 * @brief termina un thread e ne dealloca le risorse
 * @param thread e' il thread da terminare e deallocare
*/
void cleanupThread(pthread_t thread);
