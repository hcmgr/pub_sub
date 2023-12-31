//clientList.h//
//----------------------//
//This file abstracts away the functionality associated with the linked list 
//of clients that the server uses.
//----------------------//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifndef CLIENT_LIST
#define CLIENT_LIST

/* Defines the Client structure which holds all relevant information about
 * a client. Client structures are stored in linked lists with other Client's
 * depending on their topics. 
 *
 *      name - name client gave themself
 *      clientToServer - read end (from server's point of view) of socket
 *      serverToClient - write end (from client's point of view) of socket
 *      topics - string array holding the topics the client has specified
 *      isPlaceholder - (explained in init_client_list() below)
 *      next - pointer to the next client in the linked list
 *
 *
 * */
typedef struct Client {
    char* name;
    FILE* clientToServer;
    FILE* serverToClient;
    char** topics;
} Client;

/* create_client
 * -------------
 * Creates a new client from the given name and socket ends.
 *
 * name - name of client
 * clientToServer - read end of network socket (recall that Client structures
 *                  are only used by the server)
 * serverToClient - write end of network socket 
 *
 * Return:
 *      the newly created client
 *
 * */
Client* create_client(char* name, FILE* clientToServer, FILE* serverToClient);

/* print_client
 * ------------
 * Prints out the client in a readable format:
 *
 * */
void print_client(Client* client);

/* Defines the ClientListItem structure which is what we store in our linked 
 * list of clients who follow the same topic.
 *
 *      client - pointer to client we're storing
 *      next - next client in the list
 * */
typedef struct ClientListItem {
    Client* client;
    struct ClientListItem* next;
    bool isPlaceholder;
} ClientListItem;

/* init_client_list
 * ----------------
 * Initialises the head of a linked list of clients.
 *
 * NOTE: we want the list to persist even if all clients are deleted from it.
 * Therefore, if needed, we can make the head of the list a 'placeholder' 
 * client to the head is never NULL.
 *
 * placeholder - true iff client is to be a placeholder, false otherwise
 *
 * Returns:
 *      the head of a new linked list of clients
 *
 * */
ClientListItem* init_client_list(Client* client, bool isPlaceholder);

/* add_client
 * ----------
 * Adds the given client to the given linked list of clients.
 *
 * head - head of linked list 
 * client - pointer client to add
 *
 * */
void add_client(ClientListItem* head, Client* client);

/* remove_client
 * -------------
 * Removes the given client from the given linked list of clients.
 *
 * head - head of linked list
 * client - pointer to client to remove
 *
 * Returns:
 *      if client was THE HEAD of the list, returns pointer to the NEW
 *      HEAD, which is just a placeholder ListItem with no client
 *
 *      if client is in the list, but isn't the head 
 *      returns pointer to the head of the new linked list
 *
 *      else, returns NULL (client not in list)

 * */
ClientListItem* remove_client(ClientListItem* head, Client* client);

/* search 
 * ------
 * Searches for the given client in the given client list and returns
 * a pointer to the ClientListItem it's held in.
 *
 * head - pointer to head of linked list of clients
 * client - client to search for
 *
 * Returns:
 *      pointer to the ClientListItem the client is held in inside the list
 *
 * */
ClientListItem* search(ClientListItem* head, Client* client);
#endif //CLIENT_LIST



