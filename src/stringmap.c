#include "stringmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* This defines the Topic structure. This is a structure to be held
 * in a linked list that stores the StringMapItem corresponding to the topic 
 * and a pointer to the next in the list.
 * */
typedef struct Topic { 
    StringMapItem* item;
    struct Topic* next;
} Topic;

/* This is a structure to define the StringMap. It simply holds the head of 
 * a linked list of Topic structures.
 * */
struct StringMap { 
    Topic* head;
};

StringMap* stringmap_init() {
    //Our stringmap is a linked list of Topic structures, each of which stores
    //a StringMapItem and a next pointer
    StringMap* sm = calloc(1, sizeof(StringMap));
    //initialise head of linked list
    Topic* head = calloc(1, sizeof(Topic));
    head->item = NULL;
    head->next = NULL;
    sm->head = head;
    return sm;
}

/* free_topic_mem
 * --------------
 * Helper function for stringmap_free() that frees all memory associated
 * with the given Topic structure.
 *
 * topic - pointer to Topic structure to free
 *
 * */
void free_topic_mem(Topic* topic) {
    if (topic && topic->item && topic->item->key) {
        free(topic->item->key);
    }
    if (topic && topic->item) {
        free(topic->item);
    }
    if (topic) {
        free(topic);
    }
}

void stringmap_free(StringMap* sm) {
    Topic* currTopic = sm->head;
    Topic* nextTopic;
    while (currTopic) {
        nextTopic = currTopic->next; 
        free_topic_mem(currTopic);
        currTopic = nextTopic;
    }
    //free map itself
    free(sm);
}

void* stringmap_search(StringMap* sm, char* key) {
    //stringMap or key is NULL
    if (!sm || !key) {
        return NULL;
    }

    Topic* currTopic = sm->head;
    while (currTopic) {
        if (currTopic->item && !strcmp(currTopic->item->key, key)) {
            return currTopic->item->item;
        }
        currTopic = currTopic->next;
    }
    //not found
    return NULL;
}

int stringmap_add(StringMap* sm, char* key, void* item) {
    //check no arguments are NULL
    if (!sm || !key || !item) {
        return 0;
    }

    //check item not alreay present
    if (stringmap_search(sm, key)) {
        return 0;
    }

    //create StringMapItem for given key and item
    StringMapItem* smi = calloc(1, sizeof(StringMapItem));
    smi->key = strdup(key);
    smi->item = item;

    //create new Topic structure to store this item in the StringMap
    Topic* topicToAdd = calloc(1, sizeof(Topic));
    topicToAdd->item = smi;
    topicToAdd->next = NULL;

    //add topic as head
    Topic* currTopic = sm->head; 
    if (!currTopic->item) {
        currTopic->item = smi;
        currTopic->next = NULL;
        free(topicToAdd);
        return 1;
    }
    while (currTopic->next) {
        currTopic = currTopic->next;
    }
    currTopic->next = topicToAdd;
    return 1;
}

int stringmap_remove(StringMap* sm, char* key) {
    if (!sm || !key) {
        return 0;
    }
    //iterate through ourselves because we need the previous Topic
    Topic* currTopic = sm->head;
    Topic* prev = NULL;
    Topic* temp = NULL;

    while (currTopic) {
        if (currTopic->item && !strcmp(currTopic->item->key, key)) {
            //remove head
            if (currTopic == sm->head) {
                sm->head = currTopic->next;
                free_topic_mem(currTopic);
                return 1;
            }
            //remove any other Topic
            prev->next = currTopic->next;
            //cleanup removed Topic
            free_topic_mem(currTopic);
        }
        temp = currTopic;
        currTopic = currTopic->next;
        prev = temp;
    }
    //not found
    return 0;
}

StringMapItem* stringmap_iterate(StringMap* sm, StringMapItem* prev) {
    //stringMap null
    if (!sm) {
        return NULL;
    }
    //return first entry
    if (!prev && sm->head) {
        return sm->head->item;
    }
    //return next entry
    Topic* currTopic = sm->head;
    while (currTopic) {
        if (currTopic->item == prev) {
            return currTopic->next ? currTopic->next->item : NULL;
        }
        currTopic = currTopic->next;
    }
    //not found
    return NULL;
}




 

