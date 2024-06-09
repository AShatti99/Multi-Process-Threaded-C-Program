#include "header/tree.h"

Node * insNode(Node * node, const char * str, long num){

    // se l'albero e' vuoto, creo un nuovo nodo e lo restituisco
    if(node == NULL){

        node = (Node *) malloc(sizeof(Node));
        if(node == NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        node->file = (char *) malloc(MAXFILENAME * sizeof(char));
        if(node->file == NULL){
            perror("malloc");
            free(node);
            exit(EXIT_FAILURE);
        }

        strncpy(node->file, str, MAXFILENAME-1);
        node->sum = num;
        node->left = NULL;
        node->right = NULL;
    }
    // se l'albero non e' vuoto, inserisco il nuovo nodo nel sottoalbero corretto 
    // in base al valore del calcolo del task
    else{

        if(num <= node->sum){
            node->left = insNode(node->left, str, num);
        }
        else{
            node->right = insNode(node->right, str, num);
        }
    }

    return node;
}

void printTree(Node * node){
    if(node != NULL){
        printTree(node->left);
        printf("%ld %s\n", node->sum, node->file);
        printTree(node->right);
    }
}

void freeTree(Node* node) {
    if (node != NULL) {
        freeTree(node->left);
        freeTree(node->right);
        free(node->file);
        free(node);
    }
}