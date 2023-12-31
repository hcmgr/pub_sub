//client.c//
//-----------//
//This file contains the main client-side functionality
//-----------//

#include "shared.h"
// #include "csse2310a4.h"
// #include "csse2310a3.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

//helpful constants
#define PORT_NUM_INDEX 1
#define NAME_INDEX 2
#define TOPICS_INDEX 3
#define NUM_NON_TOPIC_ARGS 3
#define NAME_ERROR 1
#define TOPICS_ERROR 2
#define NODE "localhost"
#define TCP 0
#define DEFAULT 0

/* Defines the Parameters structure which holds the following command line
 * arguments given to psclient:
 *
 *      service - service name/string version of the port number
 *      portnum - port to connect to that psserver is listening on
 *      clientName - name to be associated with the client
 *      topics - list of topics client wishes to subsribe to (optional)
 * */
typedef struct {
    char* service;
    int portnum;
    char* clientName;
    char** topics;
} Parameters;

/* Defines the SocketEnds structure which holds the read and write ends of 
 * the socket connecting the client to the server:
 *
 *      serverToClient - read end
 *      clientToServer - write end
 * */
typedef struct {
    FILE* serverToClient;
    FILE* clientToServer;
} SocketEnds;

//client error codes
enum ExitCodes {
    SUCCESS,
    NUM_ARGS_ERROR,
    NAME_OR_TOPICS_ERROR,
    PORT_ERROR,
    CONNECTION_CLOSED,
    ADDRESS_ERROR
};

/* general_error
 * -------------
 * For a given error (encoded by 'errorCode'), print out a descriptive
 * message and exit with the appropriate code.
 *
 * errorCode - encodes the specific error being handled
 * extraInfo - general string that used to make error messages more 
 *             specialised/descriptive (eg: port number unable to connect to)
 * nameOrTopics - 1 if handling a 'name' error, 2 if handling a 'topics' error,
 *                0 (NULL) otherwise (necessary because these errors have 
 *                the same exit code)
 *
 * */
void general_error(int errorCode, char* extraInfo, int nameOrTopics) {
    switch (errorCode) {
        //insufficient command line args
        case NUM_ARGS_ERROR:
            fprintf(stderr, "Usage: psclient portnum name [topic] ...\n");
            exit(NUM_ARGS_ERROR);
        case NAME_OR_TOPICS_ERROR:
            //invalid name argument
            if (nameOrTopics == NAME_ERROR) {
                fprintf(stderr, "psclient: invalid name\n");
            }
            //invalid topics argument(s)
            if (nameOrTopics == TOPICS_ERROR) {
                fprintf(stderr, "psclient: invalid topic\n");
            }
            exit(NAME_OR_TOPICS_ERROR);
        //unable to connect to the given port
        case PORT_ERROR:
            fprintf(stderr, "psclient: unable to connect to port %s\n",
                    extraInfo);
            exit(PORT_ERROR);
        //network connection to server closed
        case CONNECTION_CLOSED:
            fprintf(stderr, "psclient: server connection terminated\n");
            exit(CONNECTION_CLOSED);
    }
    return;
}

/* parse_command_line
 * ------------------
 * Retreives the command line arguments given to psclient and populates a 
 * Parameters structure with said arguments.
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
 *      1 - incorrect number of args received
 *      2 - invalid name or topic
 *      3 - client can't connect to given port/service
 *
 * */
Parameters parse_command_line(int argc, char** argv) {
    //check number of arguments given
    if (argc < 3) {
        general_error(NUM_ARGS_ERROR, NULL, DEFAULT);
    }

    //retreive arguments from argv
    char* service = argv[PORT_NUM_INDEX];
    char* name = argv[NAME_INDEX];
    char** topics = calloc((argc - NUM_NON_TOPIC_ARGS) + 1, sizeof(char*));

    //check all topics are validly configured (if so, add to topics array)
    for (int i = TOPICS_INDEX; i < argc; i++) {
        if (has_space_colon_newline(argv[i])){
            general_error(NAME_OR_TOPICS_ERROR, NULL, TOPICS_ERROR);
        }
        topics[i - TOPICS_INDEX] = argv[i]; 
    }

    //check name arg is validly configured
    if (has_space_colon_newline(name)) {
        general_error(NAME_OR_TOPICS_ERROR, NULL, NAME_ERROR);
    }

    //if this point is reached, args are validly configured
    //NOTE: haven't checked whether port/service can be conneceted to 
    Parameters cmdArgs;
    memset(&cmdArgs, 0, sizeof(Parameters));
    cmdArgs.service = service; 
    cmdArgs.clientName = name;
    cmdArgs.topics = topics;
    return cmdArgs;
}

/* connect_to_server
 * -----------------
 * Connects the client socket to the given node/address on the given 
 * port/service. The method populates a structure holding the read and write
 * ends of the socket.
 *
 * node - address of node to connect to
 * service - port number in our case
 *
 * Returns:
 *      a SocketEnds structure holding the read and write ends of the socket
 *
 * Exits with:
 *      3 - if the client is unable to connect to the given port/service
 * */
SocketEnds connect_to_server(char* node, char* service) {
    //get address info struct
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; //byte stream (TCP) 
    if (getaddrinfo(node, service, &hints, &ai)) { //returns 0 on success
        freeaddrinfo(ai);
        general_error(PORT_ERROR, service, DEFAULT); //exits here
    }

    //connect socket to server address
    int fd = socket(AF_INET, SOCK_STREAM, TCP);
    if (connect(fd, (struct sockaddr*)ai->ai_addr, 
            sizeof(struct sockaddr))) { //returns 0 on success

        general_error(PORT_ERROR, service, DEFAULT); // exits here
    }
    //now socket is connected to server, separate read and write streams
    int fd2 = dup(fd);
    FILE* serverToClient = fdopen(fd, "r");
    FILE* clientToServer = fdopen(fd2, "w");
    //populate SocketEnds struct
    SocketEnds fds;
    memset(&fds, 0, sizeof(SocketEnds));
    fds.serverToClient = serverToClient;
    fds.clientToServer = clientToServer;
    return fds;
}

/* send_name
 * ---------
 * sends a 'name' command to the server, thus requesting that the server 
 * name the client.
 *
 * fd - network socket
 * name - name for client to give itself
 *
 * */
void send_name(FILE* fd, char* name) {
    fprintf(fd, "name %s\n", name);
}

/* subscribe
 * ---------
 * sends a 'sub' command to the server, thus requesting that the server
 * subscribe the client to the given topic.
 *
 * fd - network socket
 * topic - topic to subscribe to 
 *
 * */
void subscribe(FILE* fd, char* topic) {
    fprintf(fd, "sub %s\n", topic);
}

/* unsubscribe
 * ---------
 * sends an 'unsub' command to the server, thus requesting that the server
 * unsubscribe the client from the given topic.
 *
 * fd - network socket
 * topic - topic to unsubscribe from
 *
 * */
void unsubscribe(FILE* fd, char* topic) {
    fprintf(fd, "unsub %s\n", topic);
}

/* publish
 * ---------
 * sends a 'pub' command to the server, thus requesting that the server
 * send the given value to all clients subscribed to the given topic.
 *
 * fd - network socket
 * topic - topic to send the value to
 * value - value to send to the other clients
 *
 * */
void publish(FILE* fd, char* topic, char* value) {
    fprintf(fd, "pub %s %s\n", topic, value);
}

/* send_starting_msgs
 * ------------------
 * Sends the first two automatic requests to the server. These are: 
 *
 *      name <name> - which names the client as per the given command line
 *                    argument
 *      sub <topic> - which subscribes to any topics specified by the user
 *                    on the command line
 *
 * fds - read and write ends of the network socket
 * cmdArgs - Parameters structure holding the given command line arguments
 *
 * */
void send_starting_msgs(SocketEnds fds, Parameters cmdArgs) {
    //send name
    send_name(fds.clientToServer, cmdArgs.clientName);
    //subscribe to each topic
    while (cmdArgs.topics[0]) {
        subscribe(fds.clientToServer, cmdArgs.topics[0]);
        cmdArgs.topics++;
    }
    fflush(fds.clientToServer);
}

/* print_lines_loop
 * ----------------
 * Reads from the given network socket and outputs what it receives to 
 * stdout.
 *
 * serverToClient - network socket to listen on 
 *
 * */
void print_lines_loop(FILE* serverToClient) {
    char* line;
    //keeps reading from the network socket until EOF is detected (server
    //disconnects)
    while ((line = read_line(serverToClient))) {
        printf("%s\n", line);
        fflush(stdout);
        fflush(serverToClient);
    }
    general_error(CONNECTION_CLOSED, NULL, DEFAULT);
}

/* send_lines_loop
 * ---------------
 * A thread that reads a line from stdin and sends what it receives to the 
 * given network socket.
 *
 * arg - void* version of a network socket
 *
 * */
void* send_lines_loop(void* arg) {

    FILE* clientToServer = (FILE*)arg;
    char* line;
    char* lineToSend;
    while ((line = read_line(stdin))) {
        lineToSend = add_new_line(line);
        fprintf(clientToServer, lineToSend);
        fflush(clientToServer);
    }
    //EOF detected on STDIN
    exit(0);
    pthread_exit(NULL);
}

/* spawn_thread
 * ------------
 * Spawns the client thread that continously reads lines from stdin and sends
 * them to the server
 *
 * serverToClient - network socket
 *
 * */
pthread_t spawn_thread(FILE* serverToClient) {
    pthread_t threadId;
    pthread_create(&threadId, NULL, send_lines_loop, serverToClient);
    pthread_detach(threadId);
    return threadId;
}

int main(int argc, char** argv) {
    //get command line args
    Parameters cmdArgs = parse_command_line(argc, argv);
    //read and write ends of network socket
    SocketEnds fds = connect_to_server(NODE, cmdArgs.service);
    //send initial name and sub messages
    send_starting_msgs(fds, cmdArgs);
    //start 'send lines' thread
    pthread_t t = spawn_thread(fds.clientToServer);
    //start 'print lines' loop
    print_lines_loop(fds.serverToClient);
    //clean up
    pthread_join(t, NULL);
    free(cmdArgs.topics);
    exit(0);
}









