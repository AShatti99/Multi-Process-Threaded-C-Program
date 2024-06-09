#include "header/list.h"

int insTail(BinList ** l, char * s) {

    BinList * newNode = (BinList *) malloc(sizeof(BinList));
    if (newNode == NULL) {
        fprintf(stderr, "fatal error: memory allocation");
        errno = ENOMEM;
        return -1;
    }

    newNode->file = (char *) malloc(MAXFILENAME * sizeof(char));
    if (newNode->file == NULL) {
        fprintf(stderr, "fatal error: memory allocation");
        free(newNode);
        errno = ENOMEM;
        return -1;
    }

    strncpy(newNode->file, s, MAXFILENAME - 1);
    newNode->next = NULL;

    if (*l == NULL) {
        *l = newNode;
    } 
    else {
        BinList * current = *l; 
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
    return 0;
}

BinList * removeHead(BinList * l){

    // verifica se la lista e' vuota
    if(l == NULL){
        return NULL;
    }
    
    BinList * tmp = l->next;
    // libera la memoria allocata al primo nodo
    free(l->file);
    free(l);
    // restituisci il puntatore al secondo nodo che diventa la nuova testa
    return tmp;
}

void freeList(BinList *l) {
    while (l != NULL) {
        BinList *temp = l;
        l = l->next;
        free(temp->file);
        free(temp);
    }
}

void printList(BinList * l){
    BinList * current = l;
    while (current != NULL){       
        printf("file: %s\n", current->file);
        current = current->next;
    }
    printf("END LIST\n");
}