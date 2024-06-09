#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

typedef struct Node {
    char * file;
    long sum;
    struct Node* left;
    struct Node* right;
} Node;

/**
 * @brief inserisce un nodo in un albero binario in base al calcolo del task
 * @param node e' la radice dell'albero in cui inserire il nuovo nodo
 * @param str e' il il nome del file
 * @param num e' il calcolo del task 
 * @return restituisce la radice dell'albero
*/
Node * insNode(Node * node, const char * str, long num);

/**
 * @brief stampa i nodi dell'albero in ordine crescente
 * @param node e' il nodo radice dell'albero
*/
void printTree(Node * node);

/**
 * @brief libera la memoria allocata per i nodi dell'albero
 * @param node e' il nodo radice dell'albero
*/
void freeTree(Node * node);
