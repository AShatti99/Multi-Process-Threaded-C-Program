#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

typedef struct BinaryFileList{
    
    char * file;
    struct BinaryFileList * next;
} BinList;

/**
 * @brief inserisce in coda un file binario trovato come parametro
 * @param l e' l'oggetto lista
 * @param s e' il file binario preso in input
 * @return 0 in caso di successo, -1 in caso di fallimento
*/
int insTail(BinList ** l, char * s);

/**
 * @brief rimuove la testa della lista e restituisce il puntatore alla nuova testa
 * @param l e' l'oggetto lista
 * @return restituisce la lista
*/
BinList * removeHead(BinList * l);

/**
 * @brief libera la memoria associata ad ogni nodo
 * @param l e' l'oggetto lista
*/
void freeList(BinList *l);

/**
 * @brief stampa la lista (DEBUG)
 * @param l e' l'oggetto lista
*/
void printList(BinList * l);
