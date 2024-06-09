#include "header/util.h"

int check_number(const char* s) {
    if (s == NULL || strlen(s) == 0) {
        return -1;
    }

    char* e = NULL;
    errno = 0;
    int val = strtol(s, &e, 10);

    // controlla overflow/underflow
    if(errno == ERANGE) return -2;   

    // controlla se la conversione con strtol e' avvenuta con successo e che sia > 0
    if (e != NULL && *e == '\0' && val > 0) {
        return val;  // successo 
    }

    // non è un numero valido
    return -1; 
}


int check_file(const char * arg){
    struct stat info;

    if(stat(arg, &info) == -1) return -1;   // caso di errore
    if(S_ISREG(info.st_mode)) return 1;     // file regolare
    if(S_ISDIR(info.st_mode)) return 2;     // directory

    return -1;
}

int millisecondSleep(long ms){
    struct timespec req;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000;
    return nanosleep(&req, NULL);
}

int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
        if ((r=read((int)fd ,bufptr,left)) == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) return 0;   // EOF
        left    -= r;
        bufptr  += r;
    }
    return size;
}

int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
        if ((r=write((int)fd ,bufptr,left)) == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) return 0;  
        left    -= r;
        bufptr  += r;
    }
    return 1;
}

void cleanupLock(pthread_mutex_t *mtx) {
    // Deallocazione del mutex solo se e' stato inizializzato
    if(mtx != NULL) {
        pthread_mutex_destroy(mtx);
    }
}

void cleanupThread(pthread_t thread){
    pthread_kill(thread, SIGTERM);
    if(pthread_join(thread, NULL) != 0){
        fprintf(stderr, "error: pthread_join failed\n");
    }
}
