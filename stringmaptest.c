#include "stringmap.h"

#include <stdio.h>
#include <stdlib.h>

#define NUMTESTS 3

char *testKeys[NUMTESTS] = {"foo", "bar", "baz"};
void *testPtrs[NUMTESTS] = {(void *)0xdeadbeef, (void *)0x55555555, (void *)0xaaaaaaaa};

int main(int argc, char *argv[]) {
    StringMap *sm = stringmap_init();

    if(!sm) {
        exit(1);
    }

    printf("Searching empty StringMap...\n");
    for(int i=0; i<NUMTESTS; i++) {
        if(!stringmap_search(sm, testKeys[i])) {
            printf("PASS: \"%s\" not found, as expected\n", testKeys[i]);
        } else {
            printf("ERROR: \"%s\" found in empty StringMap!\n", testKeys[i]);
        }
    }

    printf("Iterating stringmap...\n");
    StringMapItem *smi = NULL;
    while((smi = stringmap_iterate(sm, smi))) {
        printf("%s:%p\n", smi->key, smi->item);
    }

    printf("Adding to empty StringMap...\n");
    for(int i=0; i < NUMTESTS; i++) {
        if(!stringmap_add(sm, testKeys[i], testPtrs[i])) {
            printf("ERROR adding (\"%s\", %p)\n", testKeys[i], testPtrs[i]);
        }
    }

    printf("Retrieving keys...\n");
    for(int i=0; i < NUMTESTS; i++) {
        void *item;
        if(!(item=stringmap_search(sm, testKeys[i]))) {
            printf("ERROR retrieving \"%s\")\n", testKeys[i]);
        } else if (item != testPtrs[i]) {
            printf("ERROR, wrong item retrieved (\"%s\", %p) != %p\n",
                    testKeys[i], testPtrs[i], item);
        } else {
            printf("PASS: \"%s\": %p == %p\n", testKeys[i], testPtrs[i], item);
        }
    }

    printf("Adding to populated StringMap...\n");
    for(int i=0; i < NUMTESTS; i++) {
        if(stringmap_add(sm, testKeys[i], testPtrs[i])) {
            printf("ERROR: adding duplicate (\"%s\", %p) succeeded\n", testKeys[i], testPtrs[i]);
        } else {
            printf("PASS: adding duplicaet \"%s\" failed\n", testKeys[i]);
        }
    }

    printf("Iterating stringmap...\n");
    smi = NULL;
    while((smi = stringmap_iterate(sm, smi))) {
        printf("%s:%p\n", smi->key, smi->item);
    }

    printf("Deleting missing key...\n");
    if(stringmap_remove(sm, "foobarbaz")) {
        printf("ERROR: deleting missing key succeeded\n");
    } else {
        printf("PASS: deleting missing key failed\n");
    }


    printf("Deleting key \"%s\"...\n", testKeys[0]);
    if(!stringmap_remove(sm, testKeys[0])) {
        printf("ERROR: stringmap_delete failed\n");
    } else {
        printf("PASS: deleting key \"%s\" succeeded\n", testKeys[0]);
    }

    printf("Iterating stringmap...\n");
    smi = NULL;
    while((smi = stringmap_iterate(sm, smi))) {
        printf("%s:%p\n", smi->key, smi->item);
    }


    printf("Searching deleted key \"%s\"...\n", testKeys[0]);
    if(stringmap_search(sm, testKeys[0])) {
        printf("ERROR: found deleted key\n");
    } else {
        printf("PASS: deleted key \"%s\" not found\n", testKeys[0]);
    }

    printf("Searching remaining keys...\n");
    for(int i=1; i < NUMTESTS; i++) {
        void *item;
        if(!(item=stringmap_search(sm, testKeys[i]))) {
            printf("ERROR retrieving \"%s\")\n", testKeys[i]);
        } else if (item != testPtrs[i]) {
            printf("ERROR, wrong item retrieved (\"%s\", %p) != %p\n",
                    testKeys[i], testPtrs[i], item);
        } else {
            printf("PASS: \"%s\": %p == %p\n", testKeys[i], testPtrs[i], item);
        }
    }

    printf("Deleting key \"%s\"...\n", testKeys[NUMTESTS-1]);
    if(!stringmap_remove(sm, testKeys[NUMTESTS-1])) {
        printf("ERROR: stringmap_delete failed\n");
    } else {
        printf("PASS: deleting key \"%s\" succeeded\n", testKeys[NUMTESTS-1]);
    }


    printf("Searching deleted key \"%s\"...\n", testKeys[NUMTESTS-1]);
    if(stringmap_search(sm, testKeys[NUMTESTS-1])) {
        printf("ERROR: found deleted key\n");
    } else {
        printf("PASS: deleted key \"%s\" not found\n", testKeys[NUMTESTS-1]);
    }

    printf("Searching remaining keys...\n");
    for(int i=0; i < NUMTESTS; i++) {
        void *item;
        if(!(item=stringmap_search(sm, testKeys[i]))) {
            printf("Unable to retrieve \"%s\"\n", testKeys[i]);
        } else if (item != testPtrs[i]) {
            printf("ERROR: wrong item retrieved (\"%s\", %p) != %p\n",
                    testKeys[i], testPtrs[i], item);
        } else {
            printf("Retrieved \"%s\": %p == %p\n", testKeys[i], testPtrs[i], item);
        }
    }

    printf("Iterating stringmap...\n");
    smi = NULL;
    while((smi = stringmap_iterate(sm, smi))) {
        printf("%s:%p\n", smi->key, smi->item);
    }

    printf("Deleting key \"%s\"...\n", testKeys[1]);
    if(!stringmap_remove(sm, testKeys[1])) {
        printf("ERROR: stringmap_delete failed\n");
    } else {
        printf("PASS: deleting key \"%s\" succeeded\n", testKeys[NUMTESTS-1]);
    }

    printf("Iterating stringmap...\n");
    smi = NULL;
    while((smi = stringmap_iterate(sm, smi))) {
        printf("%s:%p\n", smi->key, smi->item);
    }


    stringmap_free(sm);
}
