#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <signal.h>
#include <pthread.h>

#include "tree.h"
#include "util.h"

/**
 * @brief riceve richieste dai client, le inserisce in un albero e avvia un thread 
 *        per la stampa di esso
 * @return 0 in caso di successo, -1 in caso di fallimento
*/
int serverCollector();
