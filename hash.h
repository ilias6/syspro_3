#ifndef HASH
#define HASH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"
#include "heap.h"
#include "record.h"

typedef struct bucket {
  int size;
  int maxData;
  int curData;
  char * data;
} bucket;

typedef struct hashTable {
  int size;
  int bucketSize;
  bucket * table;
} hashTable;

int bIndex(hashTable, char *);
void entriesCount(bucket, int  *);
void ageStatistics(bucket, Date, char *, int **, int *);
char * i_virus(bucket, int *);
avlNode * virusTree(bucket, char *);
avlNode * countryTree(bucket, char *);
hashTable * makeTable(int, int);
unsigned long hash(char *);
void bucketInsert(bucket *, Record *, char **);
void bucketSearch(bucket, Date, Date);
void hashTableInsert(hashTable *, Record *, char ** );
void overFlow(bucket *, Record *, char **);
void freeBucket(bucket);
void freeHashTable(hashTable *);
heapNode * topKDiseasesb(heapNode *, bucket, char *, Date, Date);
heapNode * topKCountriesb(heapNode *, bucket, char *, Date, Date);
int diseaseFreqb(bucket, Date, Date, char *, char *);
int printCurrent(bucket, char *);
void printBucket(bucket);
void printTable(hashTable);

#endif
