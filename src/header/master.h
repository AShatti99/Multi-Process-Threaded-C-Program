#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 2001112L
#endif
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>

#include "list.h"


/**
 * @brief imposta la maschera dei segnali
 * @param mask puntatore alla struttura sigset_t 
 * @return 0 in caso di successo, -1 altrimenti
*/
int setSignalMask(sigset_t * mask);

/**
 * @brief aggiunge i file in input (e dalla cartella se specificata tramite la funzione
 * exploreDir) nella lista
 * @param list e' l'oggetto lista
 * @param index e' l'indice del primo file in input (optind)
 * @param numarg e' il numero totale di argomenti passati in input (argc)
 * @param args sono gli argomenti della riga di comando
 * @param namedir e' il percorso della directory da esplorare
 * @return 0 in caso di successo, -1 in caso di fallimento
*/
int fileToList(BinList ** list, int index, int numarg, char ** args, char * namedir);
