#include "header/master.h"

int setSignalMask(sigset_t * mask){

    sigemptyset(mask);
    sigaddset(mask, SIGHUP);
    sigaddset(mask, SIGINT);
    sigaddset(mask, SIGQUIT);
    sigaddset(mask, SIGTERM);
    sigaddset(mask, SIGUSR1);
    sigaddset(mask, SIGUSR2);

    // ignoro SIGPIPE 
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));    
    sa.sa_handler = SIG_IGN;
    if((sigaction(SIGPIPE,&sa,NULL)) == -1) return -1;
    
    // si applica la maschera dei segnali
    if(pthread_sigmask(SIG_BLOCK, mask, NULL) != 0) return -1;

    return 0;   // successo
}

/**
 * @brief verifica se dir termina con un punto che indica il padre 
 * @param dir e' la stringa da verificare
 * @return 1 se termina con un punto, 0 altrimenti
*/
int isdot(const char dir[]) {
  int l = strlen(dir);
  if ((l>0 && dir[l-1] == '.')) return 1; 
  return 0; 
}

/**
 * @brief esplora ricorsivamente una directory e aggiunge i file regolari trovati alla lista
 * @param namedir e' il percorso della directory da esplorare
 * @param l e' l'oggetto lista
 * @return 0 in caso di successo, -1 in caso di fallimento
*/
int exploreDir(const char * namedir, BinList ** l) {

    struct stat info;

    if (stat(namedir, &info) == -1) {
        return -1;
    }

    DIR * dir;
    if ((dir = opendir(namedir)) == NULL) {
        return -1;
    }

    struct dirent * file;
    while ((errno = 0, file = readdir(dir)) != NULL) {
        char filename[MAXFILENAME];
        int len1 = strlen(namedir);
        int len2 = strlen(file->d_name);
        if ((len1 + len2 + 2) > MAXFILENAME) {
            closedir(dir);
            return -1;
        }

        strncpy(filename, namedir, MAXFILENAME - 1);
        if (filename[len1 - 1] != '/') {
            strncat(filename, "/", MAXFILENAME - 1);
        }
        strncat(filename, file->d_name, MAXFILENAME - 1);

        if (stat(filename, &info) == -1) {
            closedir(dir);
            return -1;
        }

        if (S_ISDIR(info.st_mode)) {
            if (!isdot(filename)) {
                if (exploreDir(filename, l) == -1) {
                    closedir(dir);
                    return -1;
                }
            }
        } 
        else {
            if (check_file(filename) == 1) {
                if (insTail(l, filename) == -1) {
                    closedir(dir);
                    return -1;
                }
            }
        }
    }

    if (errno != 0) {
        closedir(dir);
        return -1;
    }

    closedir(dir);
    return 0;
}

int fileToList(BinList ** list, int index, int numarg, char ** args, char * namedir) {

    for (int i = index; i < numarg; i++) {
        int r = check_file(args[i]);
        if (r == 1) {
            if (insTail(list, args[i]) == -1) {
                return -1;
            }
        } 
        else {
            fprintf(stderr, "error: problem with file '%s'\n", args[i]);
        }
    }

    // se namedir e' diverso da end: esplora ricorsivamente la cartella
    // restituisce -1 in caso di errore
    if (strncmp(namedir, "end", MAXFILENAME-1) && exploreDir(namedir, list) == -1) {
        return -1;
    }

    //printf("file:\n");
    //printList(*list);
    //printf("\n");
    return 0;
}