#include "header/tpool.h"

// variabili definite in main.c
extern volatile sig_atomic_t usr2_rcv;
extern pthread_mutex_t usr2_lock;


/**
 * @brief invia il nome del file e il calcolo del task al collector
 * @param file e' il nome del file
 * @param sum e' il calcolo del task
*/
void sendToCollector(char * file, long sum){ 
    int client_fd;
    struct sockaddr_un server_addr;

    // creo la socket
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKNAME, sizeof(server_addr.sun_path) - 1);

    // tentativi di connessione al server
    int maxAttempt = 5;   // numero massimo di tentativi
    int attempt = 0;      // tentativi
    int connected = 0;    // flag di connessione

    while (attempt < maxAttempt && !connected) {
        // connessione al server
        if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == 0) {
            connected = 1;
        } else {
            //printf("server not ready, waiting...\n");
            sleep(1); // attendi un secondo prima di ritentare
        }
        attempt++;
    }

    // verifica se la connessione è stata stabilita
    if (!connected) {
        fprintf(stderr, "failed to connect to the server after %d attempts\n", maxAttempt);
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // invio della stringa al server
    size_t str_len = strlen(file);
    int result = writen(client_fd, &str_len, sizeof(size_t));
    if (result != 1) {
        perror("writen");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    result = writen(client_fd, file, str_len);
    if (result != 1) {
        perror("writen");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // invio del numero al server
    result = writen(client_fd, &sum, sizeof(long));
    if (result != 1) {
        perror("writen");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    close(client_fd);
}

/**
 * @brief esegue il task: effettua il calcolo
 * @param file e' il nome del file 
*/
void execution_task(char * file){

    FILE * inp = NULL;
    long sum = 0, i = 0, n;

    // apertura del file in modalita' lettura
    if((inp = fopen(file, "r")) == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // lettura dei dati del file e calcolo della somma
    while(fread(&n, sizeof(long), 1, inp) == 1){
        sum += (i * n);
        i++;
    }

    // controlla se si e' verificato un errore durante la lettura del file
    if(ferror(inp)){
        perror("fread");
        fclose(inp);
        exit(EXIT_FAILURE);
    }

    //printf("file: %s sum: %ld\n", file, sum);
    sendToCollector(file, sum);

    // chiude il file
    fclose(inp);
}

/**
 * @brief un thread del pool ottiene un task dalla coda
 * @param p e' l'oggetto thread pool
*/
void * worker_task(void * p){
    
    tpool * pool = (tpool *) p;
    LOCK(&(pool->lock));

    // loop infinito
    for(;;){

        // se la coda e' vuota e il pool non sta uscendo, aspetto
        while(!pool->queue_task && !pool->exiting){
            WAIT(&(pool->empty), &(pool->lock));
        }

        // se non ci sono task nella coda e il pool sta uscendo, esci
        if(!pool->queue_task && pool->exiting == 1){
            break;
        }

        // ottieni il task e aggiorna i puntatori della coda
        char * task = pool->queue[pool->head];
        pool->head = (pool->head + 1) % pool->queue_max_size;
        pool->queue_task--;

        // esegui il task
        execution_task(task);
        UNLOCK(&(pool->lock));
        

        // segnala che la coda non e' piu' piena
        SIGNAL(&(pool->full));
        LOCK(&(pool->lock));

        // se e' stato ricevuto SIGUSR2 
        LOCK(&usr2_lock);
        if(usr2_rcv){
            // e il numero dei thread nel pool e' > 1
            if(pool->numthread > 1){
                // decrementa il numero di thread nel pool
                usr2_rcv = 0;
                pool->numthread--;
                //resize dell'array threads 
                pool->threads = realloc(pool->threads, pool->numthread * sizeof(pthread_t));
                if(pool->threads == NULL) {
                    perror("realloc");
                    UNLOCK(&usr2_lock);
                    UNLOCK(&(pool->lock));
                    return NULL;
                }

                UNLOCK(&usr2_lock);
                UNLOCK(&(pool->lock));
                return NULL;
            }
            // alrimenti resetta semplicemente il flag
            else{
                usr2_rcv = 0;
            }
        }
        UNLOCK(&usr2_lock);
    }
    
    UNLOCK(&(pool->lock));
    return NULL;
}

tpool * createThreadPool(int numThreadInit, int pending_size){

    tpool * pool = malloc(sizeof(tpool));
    if(pool == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    // condizioni iniziali
    pool->numthread = numThreadInit;
    pool->queue_max_size = pending_size;
    pool->queue_task = 0;
    pool->head = pool -> tail = 0;
    pool->exiting = 0;

    // allocazione dei thread del pool
    pool->threads = malloc(numThreadInit * sizeof(pthread_t));
    if(pool->threads == NULL){
        free(pool);
        errno = ENOMEM;
        return NULL;
    }

    // allocazione della coda del pool
    pool->queue = malloc(pending_size * sizeof(pthread_t));
    if(pool->queue == NULL){
        free(pool->threads);
        free(pool);
        errno = ENOMEM;
        return NULL;
    }

    // allocazione di memoria per ogni elemento della coda 
    for(int i = 0; i<pending_size; i++){
        pool->queue[i] = malloc(MAXFILENAME * sizeof(char));
        if(pool->queue[i] == NULL){
            // in caso di errore nella creazione di un elemento della coda, dealloca la memoria allocata finora
            for(int j = 0; j < i; j++){
                free(pool->queue[j]);
            }
            free(pool->queue);
            free(pool->threads);
            free(pool);
            errno = ENOMEM;
            return NULL;
        }
    }

    // inizializzazione della lock e delle variabile di condizione del pool
    if ((pthread_mutex_init(&(pool->lock), NULL) != 0) || (pthread_cond_init(&(pool->empty), NULL) != 0) || (pthread_cond_init(&(pool->full), NULL) != 0))  {
        for(int i = 0; i < pending_size; i++){
            free(pool->queue[i]);
        }
        free(pool->queue);
        free(pool->threads);
        free(pool);
        errno = EFAULT;
        return NULL;
    }

    // creazione dei thread del pool
    for(int i = 0; i< numThreadInit; i++){
        if(pthread_create(&(pool->threads[i]), NULL, worker_task, (void *)pool) != 0){
            destroyThreadPool(pool);
            errno = EFAULT;
            return NULL;
        }
    }

    return pool;
}

void createNewThread(tpool *pool) {

    LOCK(&(pool->lock));

    // aumento il numero di thread nel pool
    pool->numthread++;
    // resize dell'array threads 
    pool->threads = realloc(pool->threads, pool->numthread * sizeof(pthread_t));
    if(pool->threads == NULL){
        perror("realloc");
        UNLOCK(&(pool->lock));
        exit(EXIT_FAILURE);
    }

    // creo il nuovo thread e lo aggiungo al pool
    if(pthread_create(&(pool->threads[pool->numthread - 1]), NULL, worker_task, (void *) pool) != 0){
        perror("pthread_create");
        UNLOCK(&(pool->lock));
        exit(EXIT_FAILURE);
    }

    UNLOCK(&(pool->lock));
}

void addTaskToThreadPool(tpool * pool, char * str){
    
    LOCK(&(pool->lock));

    // se la coda e' piena aspetto
    while(pool->queue_task >= pool->queue_max_size){
        WAIT(&(pool->full), &(pool->lock));
    }

    // se il pool non e' in fase di uscita, aggiungi il task
    if(!pool->exiting){
        strncpy(pool->queue[pool->tail], str, MAXFILENAME-1);
        pool->tail = (pool->tail +1) % pool->queue_max_size;
        pool->queue_task++;
    }
    
    UNLOCK(&(pool->lock));
    // segnala che la coda non e' piu' vuota
    SIGNAL(&(pool->empty));
}

/**
 * @brief scrive il numero dei threads nel pool su un file
 * @param num e' il numero dei threads nel pool
*/
void writeNumber(int num){

    FILE * file = NULL; 

    if((file = fopen("nworkeratexit.txt", "w")) == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "Threads in pool: %d\n", num);

    // controlla se si e' verificato un errore durante la scrittura del file
    if (ferror(file)) {
        fprintf(stderr, "fatal error: writing to file\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}


int destroyThreadPool(tpool * pool){

    LOCK(&(pool->lock));

    // il pool sta terminando
    pool->exiting = 1;

    // risveglio i thread in attesa della coda vuota
    BROADCAST(&(pool->empty))

    UNLOCK(&(pool->lock));
    
    // attendo il termine di tutti i thread nel pool
    for(int i = 0; i < pool->numthread; i++){
        if(pthread_join(pool->threads[i], NULL) != 0){
            errno = EFAULT;
            return -1;
        }
    }
    // segnala al collector la chiusura
    sendToCollector("end", -1);
    
    // dealloca la memoria per la coda
    for(int i = 0; i < pool->queue_max_size; i++){
        free(pool->queue[i]);
    }
    free(pool->queue);
    
    // dealloca la memoria per l'array di thread
    free(pool->threads);
    
    // scrive il numero di thread nel pool
    writeNumber(pool->numthread);

    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->full));
    pthread_cond_destroy(&(pool->empty));

    // dealloca la memoria per il pool
    free(pool);
    
    return 0;   // successo
}
