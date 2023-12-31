//server.c//
//------------//
//This file contains the main server-side functionality
//------------//

//our own source files
#include "clientList.h"
#include "shared.h"
#include "stringmap.h"
#include "stats.h"
#include "lock.h"

//normal libraries
// #include "csse2310a4.h"
// #include "csse2310a3.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <limits.h>

//useful constants
#define INVALID_NUM -1
#define MIN_NUM_ARGS 2
#define MAX_NUM_ARGS 3
#define CONNECTIONS_INDEX 1
#define PORTNUM_INDEX 2
#define MIN_PORT 1024
#define MAX_PORT 65535
#define TCP 0
#define DEFAULT_PORT 0
#define HOST_IP "localhost"
#define SPACE ' '
#define NAME_CMD "name"
#define SUB_CMD "sub"
#define UNSUB_CMD "unsub"
#define PUB_CMD "pub"
#define INVALID_MSG ":invalid\n"
#define MAX_CMD_FIELDS 3
#define EMPTY_STRING ""

//server error codes
enum ErrorCodes {
    SUCCESS,
    USAGE_ERROR,
    PORTNUM_ERROR
};

/* Defines the Parameters structure which holds the following command line
 * arguments given to psserver:
 *      
 *      maxConnections - maximum number of concurrent connections to server
 *                    permitted
 *      portnum - indicates which localhost port psserver is listening on
 *      service - string version of portnum
 * */
typedef struct { 
    int maxConnections;
    int portnum;
    char* service;
} Parameters;

/* Defines the ClientThreadArgs structure which holds all arguments we
 * wish to pass to a client thread. The arguments are as follows:
 *
 *      fd - network socket
 *      stringMap - stringMap storing mappings from topic keys to linked lists
 *                  of clients subscribing to them
 *      stringMapLock - lock for the stringMap (which is shared between 
 *                      threads)
 *      stats - Stats structure storing psserver's statistics (see Stats.h)
 *      statsLock - lock for the stats data (which is shared between threads)
 *      accessLock - connection-limiting lock
 * */
typedef struct {
    int fd;
    StringMap* stringMap;
    sem_t* stringMapLock;
    Stats* stats;
    sem_t* statsLock;
    sem_t* accessLock;
} ClientThreadArgs;

/* general_error
 * -------------
 * For a given error (encoded by 'errorCode'), print out a descrptive message
 * and exit with the appropriate code.
 *
 * errorCode - encodes the specific error being handled
 * */
void general_error(int errorCode) {
    switch (errorCode) {
        case USAGE_ERROR:
            fprintf(stderr, "Usage: psserver connections [portnum]\n");
            exit(USAGE_ERROR);
        case PORTNUM_ERROR:
            fprintf(stderr, "psserver: unable to open socket for listening\n");
            exit(PORTNUM_ERROR);
    }
    return;
}

/* parse_command_line
 * ------------------
 * Retreives the command line arguments given to psserver and populates 
 * a Parameters structure with said arguments.
 *
 * NOTE: validity of all arguments is checked for 
 * 
 * argc - number of command line arguments
 * argv - array of command line arguments
 *
 * Returns:
 *      a Parameters structure populated with the given command line args
 * 
 * Exits with:
 *      1 - incorrect number of args received, connections arg not 
 *          non-negative integer or port number out of range
 * */
Parameters parse_command_line(int argc, char** argv) {
    //check number of args given is valid
    if (!(argc >= MIN_NUM_ARGS && argc <= MAX_NUM_ARGS)) {
        general_error(USAGE_ERROR); 
    }

    int portnum = DEFAULT_PORT;

    char* service = NULL;
    int maxConnections = string_to_int(argv[CONNECTIONS_INDEX]);
    
    //check connections arg is valid
    if (maxConnections == INVALID_NUM) {
        general_error(USAGE_ERROR); 
    }

    //check portnum is valid
    if (argc == MAX_NUM_ARGS) {
        service = argv[PORTNUM_INDEX];
        portnum = string_to_int(argv[PORTNUM_INDEX]);
        //not a non-negative integer
        if (portnum == INVALID_NUM) {
            general_error(USAGE_ERROR);
        }
        //not in range
        if (!(portnum == DEFAULT_PORT || 
                (portnum >= MIN_PORT && portnum <= MAX_PORT))) {
            general_error(USAGE_ERROR); 
        }
    }
    //if reached this point, all args are valid
    //NOTE: still haven't checked whether psserver can open the given port
    Parameters cmdArgs;
    memset(&cmdArgs, 0, sizeof(Parameters));
    cmdArgs.maxConnections = maxConnections;
    cmdArgs.portnum = portnum;
    //if portnum is 0, service should be NULL
    cmdArgs.service = portnum ? service : NULL;

    return cmdArgs;
}

/* open_socket
 * -----------
 * Opens a socket for the server to listen on on the provided port/service.
 * We use IPv4 addressing and the TCP protocol.
 *
 * cmdArgs - command line arguments given by user
 *
 * Returns: 
 *      fd of the socket 
 *
 * Exits with:
 *      2 - psserver unable to listen on either ephemeral port OR specific port
 * */
int open_socket(Parameters cmdArgs) {
    //get address info of port to listen on 
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; //byte stream (TCP)
    hints.ai_flags = AI_PASSIVE; //listen on all server's interfaces (IP's)
    if (getaddrinfo(HOST_IP, cmdArgs.service, &hints, &ai)) {
        //can't listen on given port
        general_error(PORTNUM_ERROR); //exits here
    }

    //create socket, allow its rapid reuse and bind to given port
    int listeningFd = socket(AF_INET, SOCK_STREAM, TCP);
    int optVal = 1; 
    setsockopt(listeningFd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(int));
    if (bind(listeningFd, (struct sockaddr*)ai->ai_addr, 
            sizeof(struct sockaddr))) {
        general_error(PORTNUM_ERROR);
    }

    //retreive which port we're listening on 
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    socklen_t addrLen = sizeof(struct sockaddr_in);
    if (getsockname(listeningFd, (struct sockaddr*) &addr, &addrLen)) {
        //can't listen on given port
        general_error(PORTNUM_ERROR); //exits here
    }

    //indicate socket is willing to accept connections
    if (listen(listeningFd, cmdArgs.maxConnections) < 0) {
        //can't listen on given port
        general_error(PORTNUM_ERROR); //exits here
    }
    
    //print port listening on and return socket fd
    fprintf(stderr, "%u\n", ntohs(addr.sin_port));
    fflush(stdout);
    return listeningFd;
}

/* handle_name_cmd
 * ---------------
 * Handles psserver receiving a 'name' command from a client.
 *
 * client - client who sent the command
 * name - name client has assigned with the command
 *
 * NOTE: all error checking completed in handle_client_msg() (see below)
 *
 * */
void handle_name_cmd(Client* client, char* name) { 
    //ignore duplicate name
    if (client->name) {
        return; 
    } 
    client->name = name;
    return;
}

/* handle_sub_cmd
 * --------------
 * Handles psserver receiving a 'sub' command from a client
 *
 * client - client who sent the command
 * cta - ClientThreadArgs structure passed to the client thread
 * topic - topic which the client wishes to subscribe to
 *
 * Returns:
 *      true iff client is successfully subscribed, false otherwise
 *
 * */
bool handle_sub_cmd(Client* client, ClientThreadArgs* cta, char* topic) { 
    //ignore if client not named already
    if (!client->name) {
        return false;
    }
    StringMapItem* currItem = NULL;
    take_lock(cta->stringMapLock);
    
    //loop through list of topics and add client to the linked list of clients 
    //subscribed to the given topic
    while ((currItem = stringmap_iterate(cta->stringMap, currItem))) {
        if (!strcmp(currItem->key, topic)) { 
            //ignore request if client already subbed
            if (search(currItem->item, client)) {
                release_lock(cta->stringMapLock);
                return false;
            }
            //add client to list of clients subbed to given topic
            add_client(currItem->item, client);            
            release_lock(cta->stringMapLock);
            //log a successful sub request
            update_stat(cta->stats, INC_SUB, cta->statsLock); 
            return true;
        }
    }
    release_lock(cta->stringMapLock);
    //topic doesn't exist - create it
    ClientListItem* head = init_client_list(client, false);
    int result = stringmap_add(cta->stringMap, topic, head);
    if (result) {
        //log successful sub request (only if topic validly added)
        update_stat(cta->stats, INC_SUB, cta->statsLock); 
    }
    return result;
}

/* handle_unsub_cmd
 * ----------------
 * Handles psserver receiving an 'unsub' command from a client.
 *
 * client - client who sent the command
 * cta - ClientThreadArgs structure that was passed to the client thread
 * topic - topic which the client wishes to unsubscribe from
 * isDisconnecting - true iff client is unsubscribing because they are 
 *                   disconnecting, false if they are just normally 
 *                   unsubscribing
 * Returns: 
 *      true iff client is successfully unsubscribed, false otherwise
 *
 * */
bool handle_unsub_cmd(Client* client, ClientThreadArgs* cta, char* topic, 
        bool isDisconnecting) { 

    //ignore if client not named already
    if (!client->name) {
        return false;
    }

    StringMapItem* currItem = NULL;
    take_lock(cta->stringMapLock);

    //loop through list of topics and remove the client from the linked list
    //of clients subscribed to the given topic
    while ((currItem = stringmap_iterate(cta->stringMap, currItem))) {
        if (!strcmp(currItem->key, topic)) {
            //remove the client (returns new linked list of clients)
            ClientListItem* newHead = remove_client(currItem->item, client);
            if (newHead) {
                //replace linked list of clients with new version
                currItem->item = newHead; 
            }
            release_lock(cta->stringMapLock);

            //log a successful sub request
            //successful if: not disconnecting and client was in the list
            if (!isDisconnecting && newHead) {
                update_stat(cta->stats, INC_UNSUB, cta->statsLock);
            }
            return newHead;
        }
    }
    release_lock(cta->stringMapLock);
    //topic doesn't exist
    return false;
}

/* handle_pub_cmd
 * --------------
 * Handles psserver receiving a publish request from a client.
 *
 * client - structure representing client who sent the command
 * cta - arguments given to the thread 
 * toks - tokenised form of the sent command
 *
 * Returns:
 *      true iff successful, false otherwise
 *      NOTE: fails if the topic specified doesn't exist
 * */
bool handle_pub_cmd(Client* client, ClientThreadArgs* cta, char** toks) {

    //ignore if client not named already 
    if (!client->name) {
        return false;
    }
    
    StringMapItem* mapItem = NULL;
    char* topic = toks[1]; //topic to publish to 
    char* value = toks[2]; //value/msg to publish

    //log successful pub command
    update_stat(cta->stats, INC_PUB, cta->statsLock);

    //loop through list of topics and publish the given value/msg to every 
    //client subscribed to the given topic
    take_lock(cta->stringMapLock);
    while ((mapItem = stringmap_iterate(cta->stringMap, mapItem))) {
        if (!strcmp(mapItem->key, topic)) {
            //loop through the linked list of clients subed to the topic
            ClientListItem* clientItem = mapItem->item;
            Client* currClient;
            while (clientItem) {
                currClient = clientItem->client;
                //NOTE: a topic with no clients is represented by a placeholder
                //client, which is just the head of an otherwise-empty list
                if (!clientItem->isPlaceholder) { 
                    fprintf(currClient->serverToClient, "%s:%s:%s\n",
                            client->name, topic, value);
                    fflush(currClient->serverToClient);
                }
                clientItem = clientItem->next;
            }
            release_lock(cta->stringMapLock);
            return true;
        }
    }
    release_lock(cta->stringMapLock);
    //topic doesn't exist
    return false;
}

/* handle_client_msg
 * -----------------
 * Processes the user-given command and handles it accordingly. 
 *
 * msg - command sent by user
 * client - structure representing client who sent the command
 * cta - arguments given to the client thread
 *
 * */
void handle_client_msg(char* msg, Client* client, ClientThreadArgs* cta) {
    //tokenised form of given message
    char** rawToks = split_line(strdup(msg), SPACE);
    //tokenised form with maximum three strings
    char** toks = split_line_max(rawToks, MAX_CMD_FIELDS);
    int toksLen = string_array_length(toks);

    //invalid number of fields
    if (toksLen < 2) {
        fprintf(client->serverToClient, INVALID_MSG);
        fflush(client->serverToClient);
        return;
    }

    //handle each of the command types
    char* cmd = toks[0];
    //name 
    if (!strcmp(cmd, NAME_CMD) && toksLen == 2 &&
            !has_space_colon_newline(toks[1])) {

        handle_name_cmd(client, toks[1]);
        fflush(client->serverToClient);
    //sub
    } else if (!strcmp(cmd, SUB_CMD) && toksLen == 2 && 
            !has_space_colon_newline(toks[1])) {

        handle_sub_cmd(client, cta, toks[1]); 
        fflush(client->serverToClient);

    //unsub
    } else if (!strcmp(cmd, UNSUB_CMD) && toksLen == 2 &&
            !has_space_colon_newline(toks[1])) {

        handle_unsub_cmd(client, cta, toks[1], false);
        fflush(client->serverToClient);

    //pub
    } else if (!strcmp(cmd, PUB_CMD) && toksLen >= 3 && 
            !has_space_colon_newline(toks[1]) && 
            strcmp(toks[2], EMPTY_STRING)) {

        handle_pub_cmd(client, cta, toks);
        fflush(client->serverToClient);

    //invalid command type
    } else {
        fprintf(client->serverToClient, INVALID_MSG);
        fflush(client->serverToClient);
    }
}

/* init_client_thread_args
 * -----------------------
 * Initialises a ClientThreadArgs structure to pass to a client thread.
 *
 * stringMap - psserver's string map
 * stats - psserver's statistics
 * numAllowed - max number of connections allowed (as specified on command
 *              line)
 * 
 * Returns:
 *      the newly created ClientThreadArgs structure
 *
 * */
ClientThreadArgs* init_client_thread_args(StringMap* stringMap, Stats* stats, 
        int numAllowed) {

    ClientThreadArgs* cta = malloc(sizeof(ClientThreadArgs));
    memset(cta, 0, sizeof(ClientThreadArgs));
    cta->stringMap = stringMap;
    cta->stats = stats;

    //stringMap lock
    sem_t* smLock = malloc(sizeof(sem_t));
    init_lock(smLock, 1);
    cta->stringMapLock = smLock;

    //stats lock
    sem_t* statsLock = malloc(sizeof(sem_t));
    init_lock(statsLock, 1);
    cta->statsLock = statsLock;

    //access lock
    sem_t* accessLock = malloc(sizeof(sem_t));
    if (numAllowed == 0) {
        init_lock(accessLock, INT_MAX);
    } else {
        init_lock(accessLock, numAllowed);
    }

    cta->accessLock = accessLock;
    return cta;
}

/* handle_client_thread
 * --------------------
 * Every time a new client joins, we spawn off a new thread which calls this
 * function. Here, we handle commands sent by the client and update the 
 * universal stats depending on what they give us.
 *
 * arg - ClientThreadArgs structure 
 *
 * Exits:
 *      -when EOF read from the client (client disconnects)
 *
 * */
void* handle_client_thread(void* arg) {
    ClientThreadArgs* cta = (ClientThreadArgs*)arg;
    //log a connected client
    update_stat(cta->stats, INC_CLIENTS_CURR, cta->statsLock);
    
    int fd = cta->fd;
    int fd2 = dup(fd);

    FILE* clientToServer = fdopen(fd, "r");
    FILE* serverToClient = fdopen(fd2, "w");
    Client* client = create_client(NULL, clientToServer, serverToClient);


    char* line;
    while ((line = read_line(clientToServer))) {
        handle_client_msg(line, client, cta);
    }

    //decrement number of current clients
    update_stat(cta->stats, DEC_CLIENTS_CURR, cta->statsLock); 
    //increment number of total clients ever connected
    update_stat(cta->stats, INC_CLIENTS_ALL, cta->statsLock); 

    //allows another waiting client to connect
    release_lock(cta->accessLock);

    fflush(stdout);
    pthread_exit(NULL);
}

/* server_infinite_loop
 * --------------------
 * This is psserver's main loop. It continually:
 *
 *      -waits for a new client connection
 *      -creates a new socket with which to communicate to the client
 *      -spawns off a new client thread (see handle_client_thread()) to deal
 *       with said client
 * 
 * listenFd - socket on which to listen for new client connections
 * cta - ClientThreadArgs structure to pass to client threads
 *
 * */
void server_infinite_loop(int listenFd, ClientThreadArgs* cta) {
    while (true) {
        //only accept new clients once max number of connections isn't exceeded
        take_lock(cta->accessLock);

        //Block waiting for a new connection
        int fd = accept(listenFd, 0, 0); 

        //set new thread's network socket fd
        cta->fd = fd;

        //spawn new thread
        pthread_t threadId;

        pthread_create(&threadId, NULL, handle_client_thread, cta);

        //ensures thread will give resouces back once terminated
        pthread_detach(threadId);
    }
}

int main(int argc, char** argv) {
    //get command line args
    Parameters cmdArgs = parse_command_line(argc, argv);
    //network socket we accept connections on
    int listeningFd = open_socket(cmdArgs);
    //initialise shared string map
    StringMap* stringMap = stringmap_init(); 
    //initialise shared statistics structure
    Stats* stats = stats_init(); 
    //initialise structure we pass to each client thread
    ClientThreadArgs* cta = init_client_thread_args(stringMap, stats, 
            cmdArgs.maxConnections);
    //initialise structure we pass to our separate SIGHUP/stats thread
    StatsThreadArgs* sta = init_stats_thread_args(cta->stats, cta->statsLock);
    //start SIGHUP/stats thread
    start_statistics_thread(sta);
    //main loop of server
    server_infinite_loop(listeningFd, cta);
}

/* ALL DYNAMICALLY ALLOCATED MEMORY USED IN THIS PROJECT:
 *
 * NOTE: kept track of this for own purposes
 *
 * server.c:
 *      -ClientThreadArgs
 *          -all malloc'd memory in threadArgs is shared 
 *           (sm, stats, smLock, statsLock, accessLock)
 *          -therefore, only free once SERVER terminates
 * shared.c:
 *      -add_new_line()
 *          -string we return is malloc'd
 *      -split_line_max()
 *          -array of tokens we return
 * stats.c:
 *      -stats_init()
 *          -what we return
 *      -init_stats_thread_args()
 *          -sta we return
 *          -signal mask (sta->signalMask)
 *
 * clientList.c:
 *      -create_client()
 *          -client we return
 *      -init_client_list()
 *          -Client*  we return (HEAD of list)
 *      -add_client()
 *          -Client* we add
 *              -therefore, every linked list of clients is entirely made up
 *               of malloc'd memory
 * */


