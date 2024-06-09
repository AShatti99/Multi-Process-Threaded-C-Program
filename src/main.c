#define _POSIX_C_SOURCE 2001112L
#include <getopt.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <wait.h>

#include "header/util.h"
#include "header/master.h"
#include "header/tpool.h"
#include "header/collector.h"

tpool * pool;
volatile sig_atomic_t stop = 0;
volatile sig_atomic_t usr2_rcv = 0;
pthread_mutex_t usr2_lock;

static void *sigHandler(void * arg){

    int sig;
    while(!stop){
        
        if(sigwait((sigset_t*) arg, &sig) != 0){
            perror("fatal error: sigwait");
            return NULL;
        }

        switch (sig){
            case SIGHUP: 
            case SIGINT:
            case SIGQUIT: 
            case SIGTERM: 
                //write(1, " ==========> end signal handler <==========\n", 45);
                stop = 1;
                break;
            case SIGUSR1:
                //write(1, "==========> received SIGUSR1 <==========\n", 42);
                createNewThread(pool);
                break;
            case SIGUSR2: 
                LOCK(&usr2_lock);
                usr2_rcv = 1;
                UNLOCK(&usr2_lock);
                //write(1, "==========> received SIGUSR2 <==========\n", 42);
                break;
            default:;
        }
    }

    return NULL;
}

int main(int argc, char *argv[]){

    if(argc < 2){
        USAGE(argv[0]);
        return -1;
    }
    
    // valori di default
    int nthread = 4, qlen = 8, delay = 0;
    // nome della cartella presa in input, se rimane "end" significa 
    // che non e' stata passata nessuna cartella
    char namedir[MAXFILENAME] = "end";

    // ---------------------------- VERIFICO I PARAMETRI IN INPUT ------------------------------
    int opt;
    while ((opt = getopt(argc, argv, ":n:q:d:t:")) != -1){

        switch (opt){

            // numero di thread worker
            case 'n':
                nthread = check_number(optarg);
                if(nthread == -1){
                    fprintf(stderr, "fatal error: '%s' invalid thread worker number\n", optarg);
                    return -1;
                }
                break;
                
            // lunghezza coda concorrente
            case 'q':
                qlen = check_number(optarg);
                if(qlen == -1){
                    fprintf(stderr, "fatal error: '%s' invalid queue length\n", optarg);
                    return -1;
                }
                break;
            
            // directory contenente file binari
            case 'd':
                // se e' una cartella copio il nome in namedir
                if(check_file(optarg) != 2){
                    fprintf(stderr, "fatal error: '%s' check the specified path and access permissions\n", optarg);
                    return -1;
                }
                strncpy(namedir, optarg, MAXFILENAME-1);
                break;
            
            // tempo in millisecondi tra inserimento di due task consecutivi
            case 't': 
                delay = check_number(optarg);
                if(delay == -1){
                    fprintf(stderr, "fatal error: '%s' invalid delay\n", optarg);
                    return -1;
                }
                break;

            case ':': fprintf(stderr, "error: option '%c' requires an argument\n", optopt); USAGE(argv[0]); break;
            case '?': fprintf(stderr, "error: option '%c' unrecognized\n", optopt); USAGE(argv[0]) break;
            default:;
        }
    }
    // se non vi sono file in ingresso o una cartella, esco
    if(optind == argc && !strncmp(namedir, "end", MAXFILENAME-1)){
        fprintf(stderr, "fatal error: missing file or directory input\n");
        return -1;
    }
    
    // maschero i segnali
    sigset_t mask;
    if(setSignalMask(&mask) == -1){
        fprintf(stderr, "fatal error: unable to configure signal handling\n");
        return -1;
    }

    pid_t pid;
    if((pid = fork()) == -1){
        perror("fork");
        return -1;
    }

    // Collector
    if(pid == 0){
        if(serverCollector() != 0) return -1;
    }
    // MasterWorker
    else{

        if((pthread_mutex_init(&(usr2_lock), NULL) != 0)){
            perror("pthread_mutex_init");
            return -1;
        }

        pthread_t sighandlerTh;
        if(pthread_create(&sighandlerTh, NULL, sigHandler, (void *)&mask) != 0){
            perror("pthread_create");
            cleanupLock(&usr2_lock);
            return -1;
        }

        // creo il threadPool
        pool = createThreadPool(nthread, qlen);
        if(pool == NULL){
            fprintf(stderr, "fatal error: unable to create thread pool\n");
            cleanupThread(sighandlerTh);
            cleanupLock(&usr2_lock);
            return -1;
        }

        // creo una lista dove metterci i file in input
        BinList * list = NULL;
        if(fileToList(&list, optind, argc, argv, namedir) != 0){
            destroyThreadPool(pool);
            cleanupThread(sighandlerTh);
            cleanupLock(&usr2_lock);
            return -1;
        }

        // passo gli elementi della lista al thread pool
        BinList * tmp = NULL;
        while (list != NULL && !stop){
            addTaskToThreadPool(pool, list->file);
            millisecondSleep(delay);
            tmp = removeHead(list);
            list = tmp;
        }
        
        // libero le risorse
        freeList(list);
        cleanupThread(sighandlerTh);
        if(destroyThreadPool(pool) == -1){
            fprintf(stderr, "fatal error: unable to destroy thread pool\n");
            cleanupLock(&usr2_lock); 
            return -1;
        }
        if(waitpid(pid, NULL, 0) == -1){
            perror("waitpit");
            cleanupLock(&usr2_lock);   
            unlink(SOCKNAME);
            return -1;
        }
        cleanupLock(&usr2_lock);   
        unlink(SOCKNAME);
    }
    

    return 0;
}
