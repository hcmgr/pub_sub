//clientList.c//
//----------------------//
//This file abstracts away the functionality associated with the linked list 
//of clients that the server uses.
//----------------------//

#include <stdlib.h>
#include <stdio.h>
#include "clientList.h"
#include <string.h>
#include <stdbool.h>

Client* create_client(char* name, FILE* clientToServer, FILE* serverToClient) {
    Client* client = malloc(sizeof(Client));
    client->name = name;
    client->clientToServer = clientToServer;
    client->serverToClient = serverToClient;
    return client;
}

void print_client(Client* client) {
    printf("name: %s\n", client->name);
}

ClientListItem* init_client_list(Client* client, bool isPlaceholder) {
    ClientListItem* head = malloc(sizeof(ClientListItem));
    head->isPlaceholder = isPlaceholder;
    head->client = client;
    return head;
}

void add_client(ClientListItem* head, Client* client) {
    //construct ClientListItem for new client
    ClientListItem* li = calloc(1, sizeof(ClientListItem));
    li->client = client;
    //add client to end of list
    ClientListItem* curr = head;
    while (curr->next) {
        curr = curr->next;
    }
    if (curr->isPlaceholder) {
        curr->client = client;
        curr->isPlaceholder = false;
        free(li);
    } else {
        curr->next = li;
    }
}

ClientListItem* remove_client(ClientListItem* head, Client* client) {
    //special case of removing head
    if (head->client == client) {
        //IMPORTANT: must distinguish between:
        // (a) removing only person in list (NULL returned) AND;
        // (b) the given client not being in the list (NULL returned)
        // Thus, if head->next is NULL, initialise list head as a placeholder
        ClientListItem* newHead = head->next;
        return newHead ? newHead : init_client_list(client, true);
    }

    //find client in list 
    ClientListItem* prev = head;
    ClientListItem* curr = head->next;
    ClientListItem* temp;
    while (curr) {
        if (curr->client == client) {
            prev->next = curr->next;
            return head;
        }
        temp = curr;
        curr = curr->next;
        prev = temp;
    }
    //client wasn't in list
    return NULL;
}

ClientListItem* search(ClientListItem* head, Client* client) {
    ClientListItem* curr = head;
    while (curr) {
        if (curr->client == client && !curr->isPlaceholder) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}
