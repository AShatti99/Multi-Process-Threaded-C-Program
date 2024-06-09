#include "header/collector.h"

#define DELAY 1000  // un secondo
volatile sig_atomic_t out = 0;

/**
 * @brief stampa periodicamente l'albero utilizzando un intervallo di tempo specificato
 * @param arg e' il puntatore all'albero
*/
void *printTreePeriodically(void *arg) {
    Node *tree = (Node *)arg;
    while (!out) {
        // Attendi DELAY secondi prima di stampare di nuovo l'albero
        millisecondSleep(DELAY); 
        //printf("---------------> Tree:\n");
        if(!out) printTree(tree);
    }
    return NULL;
}

int serverCollector(){
    
    int listenfd, connfd;                   
    struct sockaddr_un server_addr;
    // insieme di descrittori per la select
    fd_set read_fds, tmpset;
    // buffer per memorizzare i dati ricevuti dal client
    char buffer[MAXFILENAME];
    pthread_t printTreeThread;
    Node * tree = NULL;
    int start = 0;

    // creo la socket per l'ascolto delle connesioni dei client
    listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        return -1;
    }

    // inizializzo la struttura per l'indirizzo della socket
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKNAME, sizeof(server_addr.sun_path) - 1);

    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        close(listenfd);
        return -1;
    }

    // avvio dell'ascolto delle connessioni
    if (listen(listenfd, 1) == -1) {
        perror("listen");
        close(listenfd);
        return -1;
    }

    // inziializzo l'insieme dei descrittori per la select
    FD_ZERO(&read_fds);
    FD_SET(listenfd, &read_fds);

    while (1) {

        tmpset = read_fds;

        if (select(FD_SETSIZE, &tmpset, NULL, NULL, NULL) == -1) {
            perror("select");
            close(listenfd);
            return -1;
        }

        // controllo se ci sono nuove connessioni in arrivo
        if (FD_ISSET(listenfd, &tmpset)) {
            // accetto la connessione
            if ((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1) {
                perror("accept");
                continue;
            }

            // ricevo la stringa dal client
            size_t str_len;
            int result = readn(connfd, &str_len, sizeof(size_t));
            if (result == -1) {
                perror("readn");
                close(connfd);
                continue;
            }
            if (result == 0) {
                fprintf(stderr, "error: EOF during string length reading\n");
                close(connfd);
                continue;
            }

            result = readn(connfd, buffer, str_len);
            if (result == -1) {
                perror("readn");
                close(connfd);
                continue;
            }
            if (result == 0) {
                fprintf(stderr, "error: EOF during string reading\n");
                close(connfd);
                continue;
            }
            buffer[str_len] = '\0';

            // ricevo la somma dal client
            long num;
            result = readn(connfd, &num, sizeof(long));
            if (result == -1) {
                perror("readn");
                close(connfd);
                continue;
            }
            if (result == 0) {
                fprintf(stderr, "error: EOF during number reading\n");
                close(connfd);
                continue;
            }

            // se ho ricevuto la stringa di terminazione, killo il thread della stampa ed esco
            if(strncmp(buffer, "end", 3) == 0){
                if(start) {
                    out = 1; 
                    pthread_join(printTreeThread, NULL);
                }
                break;
            }

            //printf("%d %s\n", number, buffer);

            // inserisco nell'albero binario il file name e il calcolo del task
            tree = insNode(tree, buffer, num);

            // faccio partire il thread di stampa
            if(!start){
                if (pthread_create(&printTreeThread, NULL, printTreePeriodically, (void *)tree) != 0) {
                    perror("pthread_create");
                    return -1;
                }
                start = 1;
            }

            // chiudo la connessione
            close(connfd);
        }
    }

    // prima di uscire stampo l'albero
    printTree(tree);
    // dealloco l'albero
    freeTree(tree);
    // chiusura della socket
    close(listenfd);
    return 0;
}
