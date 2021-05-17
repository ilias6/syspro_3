#include <stdio.h>
#include <stdlib.h>
#include "hash.h"
#include "heap.h"
#include "avl.h"

/*A hash function*/
unsigned long hash(char * str) {
  unsigned long hash = 5381;
  int c;

  while (c = *str++)
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

int bIndex(hashTable t, char * str) {
  return hash(str)%t.size;
}

/*If the bucket is full, this function is called*/
/*and creates a new one to the chain*/
void overFlow(bucket * b, Record * r, char ** str) {
  /*Inializing the new buffer*/
  bucket * nextB = malloc(sizeof(bucket));
  nextB->size = b->size;
  nextB->maxData = b->maxData;
  nextB->curData = 1;
  nextB->data = malloc(nextB->size);

  /*Marking the end, showing that there is not a next bucket*/
  int endOfChain = -1;
  memcpy(nextB->data+nextB->maxData*2*sizeof(char *), &endOfChain, sizeof(int));

  /*Creating the entry's tree*/
  avlNode * tree = NULL;
  tree = avlInsert(tree, r);
  memcpy(nextB->data, str, sizeof(char *));
  memcpy(nextB->data+sizeof(char *), &tree, sizeof(char *));

  memcpy(b->data+b->maxData*2*sizeof(char *), &nextB, sizeof(char *));

}

/*For globalDiseaseStats*/
void bucketSearch(bucket b, Date d1, Date d2) {
  avlNode * tree = NULL;
  char * virus = NULL;
  int count = 0;
  for (int i = 0; i < b.curData; ++i) {
    memcpy(&tree, b.data+i*2*sizeof(char *)+sizeof(char *), sizeof(char *));
    count = avlSearch(tree, d1, d2, "\0", "\0");
    memcpy(&virus, b.data+i*2*sizeof(char *), sizeof(char *));
    printf("%s %d\n", virus, count);
  }

  /*Go to the next bucket if there is one*/
  bucket * bptr = NULL;
  int next = 0;
  memcpy(&next, b.data+b.maxData*2*sizeof(char *), sizeof(int));
  if (b.curData == b.maxData && next != -1) {
    memcpy(&bptr, b.data+sizeof(char *)*2*b.maxData, sizeof(char *));
    bucketSearch(*bptr, d1, d2);
  }
}

void bucketInsert(bucket * b, Record * r, char ** str) {
  char * str2 = NULL;
  avlNode * tree = NULL;
  for (int i = 0; i < b->curData; ++i) {
    memcpy(&str2, b->data+i*2*sizeof(char *), sizeof(char *));
    /*If this hash value exists*/
    if (!strcmp(str2, *str)) {
      memcpy(&tree, b->data+i*2*sizeof(char *)+sizeof(char *), sizeof(char *));
      tree = avlInsert(tree, r);
      memcpy(b->data+i*2*sizeof(char *)+sizeof(char *), &tree, sizeof(char *));
      return;
    }
  }

  int nextB = 0;
  memcpy(&nextB, b->data+b->maxData*2*sizeof(char *), sizeof(int));
  /*If there is next bucket*/
  if (nextB != -1) {
    bucket * bptr = NULL;
    memcpy(&bptr, b->data+b->maxData*2*sizeof(char *), sizeof(char *));
    bucketInsert(bptr, r, str);
    return;
  }
  /*Else create a new one*/
  if (b->curData >= b->maxData) {
    overFlow(b, r, str);
    return;
  }
  /*There is space left in this bucket*/
  memcpy(b->data+b->curData*2*sizeof(char *), str, sizeof(char *));
  tree = avlInsert(tree, r);
  memcpy(b->data+b->curData*2*sizeof(char *)+sizeof(char *), &tree, sizeof(char *));

  b->curData++;
}

void hashTableInsert(hashTable * t, Record * r, char ** str) {
  int index = bIndex(*t, *str);
  bucketInsert(&t->table[index], r, str);
}

bucket makeBucket(int size) {
  bucket b;
  b.size = size;
  b.curData = 0;
  /*Each pair of Data include a pointer to the string that is hashed*/
  /*and a pointer to the string's tree*/
  b.maxData = (size-sizeof(char *)) / sizeof(char *) / 2;
  if (b.maxData <= 0) {
    printf("Error while making buckets :\nNeed more space!\n");
    exit(1);
  }
  b.data = malloc(b.size);

  int endOfChain = -1;
  memcpy(b.data+b.maxData*2*sizeof(char *), &endOfChain, sizeof(int));

  return b;
}

hashTable * makeTable(int tableSize, int bucketSize) {
  hashTable * t = malloc(sizeof(hashTable));
  t->size = tableSize;
  t->bucketSize = bucketSize;
  t->table = malloc(t->size*sizeof(bucket));
  for (int i = 0; i < t->size; ++i)
    t->table[i] = makeBucket(bucketSize);

  return t;
}

int diseaseFreqb(bucket b, Date d1, Date d2, char * virus, char * country) {
  char * diseaseID = NULL;
  avlNode * tree = NULL;
  /*Search the current bucket*/
  for (int i = 0; i < b.curData; ++i) {
    memcpy(&diseaseID, b.data+i*2*sizeof(char *), sizeof(char *));
    if (!strcmp(diseaseID, virus)) {
      memcpy(&tree, b.data+i*2*sizeof(char *)+sizeof(char *), sizeof(char *));
      return avlSearch(tree, d1, d2, country, "\0");
    }
  }
  /*If nothing is found check for the next bucket*/
  bucket * bptr = NULL;
  int next = 0;
  memcpy(&next, b.data+b.maxData*2*sizeof(char *), sizeof(int));
  if (b.curData == b.maxData && next != -1) {
    memcpy(&bptr, b.data+sizeof(char *)*2*b.maxData, sizeof(char *));
    diseaseFreqb(*bptr, d1, d2, virus, country);
  }
  /*Searched all the buckets with this hash value!*/
  else if (next == -1 && virus != NULL) {
    printf("There is no patient with this virus!\n");
    return 0;
  }
}

void ageStatistics(bucket b, Date d, char * country, int ** stats, int * count) {
  avlNode * tree = NULL;
  char * virus = NULL;

  for (int i = 0; i < b.curData; ++i) {
    memcpy(&virus, b.data+i*2*sizeof(char *), sizeof(char *));
    memcpy(&tree, b.data+i*2*sizeof(char *)+sizeof(char *), sizeof(char *));
    ageSearch(tree, d, country, virus, stats[*count]);
    (*count)++;
  }

  bucket * bptr = NULL;
  int next = 0;
  memcpy(&next, b.data+b.maxData*2*sizeof(char *), sizeof(int));
  if (b.curData == b.maxData && next != -1) {
    memcpy(&bptr, b.data+sizeof(char *)*2*b.maxData, sizeof(char *));
    ageStatistics(*bptr, d, virus, stats, count);
  }
}

avlNode * countryTree(bucket b, char * country) {
  char * Bcountry = NULL;
  for (int j = 0; j < b.curData; ++j) {
    memcpy(&Bcountry, b.data+j*2*sizeof(char *), sizeof(char *));
    if (!strcmp(country, Bcountry)) {
      avlNode * tree = NULL;
      memcpy(&tree, b.data+j*2*sizeof(char *)+sizeof(char *), sizeof(char *));
      return tree;
    }
  }
  int next = 0;
  bucket * bptr = NULL;
  memcpy(&next, b.data+b.maxData*2*sizeof(char *), sizeof(int));
  if (b.curData == b.maxData && next != -1) {
    memcpy(&bptr, b.data+sizeof(char *)*2*b.maxData, sizeof(char *));
    return countryTree(*bptr, country);
  }
  return NULL;
}

avlNode * virusTree(bucket b, char * virus) {
  char * disease = NULL;
  for (int j = 0; j < b.curData; ++j) {
    memcpy(&disease, b.data+j*2*sizeof(char *), sizeof(char *));
    if (!strcmp(virus, disease)) {
      avlNode * tree = NULL;
      memcpy(&tree, b.data+j*2*sizeof(char *)+sizeof(char *), sizeof(char *));
      return tree;
    }
  }
  int next = 0;
  bucket * bptr = NULL;
  memcpy(&next, b.data+b.maxData*2*sizeof(char *), sizeof(int));
  if (b.curData == b.maxData && next != -1) {
    memcpy(&bptr, b.data+sizeof(char *)*2*b.maxData, sizeof(char *));
    return virusTree(*bptr, virus);
  }
  return NULL;
}

char * i_virus(bucket b, int * i) {
  char * virus = NULL;
  for (int j = 0; j < b.curData; ++j) {
    if (*i == 0) {
      memcpy(&virus, b.data+j*2*sizeof(char *), sizeof(char *));
      return virus;
    }
    (*i)--;
  }
  int next = 0;
  bucket * bptr = NULL;
  memcpy(&next, b.data+b.maxData*2*sizeof(char *), sizeof(int));
  if (b.curData == b.maxData && next != -1) {
    memcpy(&bptr, b.data+sizeof(char *)*2*b.maxData, sizeof(char *));
    return i_virus(*bptr, i);
  }
  return NULL;
}

void entriesCount(bucket b, int  * count) {
  *count += b.curData;

  bucket * bptr = NULL;
  int next = 0;
  memcpy(&next, b.data+b.maxData*2*sizeof(char *), sizeof(int));
  if (b.curData == b.maxData && next != -1) {
    memcpy(&bptr, b.data+sizeof(char *)*2*b.maxData, sizeof(char *));
    entriesCount(*bptr, count);
  }
}

/*Returns: (0) if the disease is found or all the diseases are printed
           (-1)if the given disease is not found
*/
int printCurrent(bucket b, char * str) {
  char * diseaseID = NULL;
  avlNode * tree = NULL;
  Date d = {-1};
  for (int i = 0; i < b.curData; ++i) {
    memcpy(&diseaseID, b.data+i*2*sizeof(char *), sizeof(char *));
    /*If disease is given and is found*/
    if (str != NULL && !strcmp(diseaseID, str)) {
      memcpy(&tree, b.data+i*2*sizeof(char *)+sizeof(char *), sizeof(char *));
      printf("%s %d\n", diseaseID, avlSearch(tree, d, d, "\0", "\0"));
      return 0;
    }
    /*if there is not any disease given*/
    else if (str == NULL){
      memcpy(&tree, b.data+i*2*sizeof(char *)+sizeof(char *), sizeof(char *));
      printf("%s %d\n", diseaseID, avlSearch(tree, d, d, "\0", "\0"));
    }
  }


  bucket * bptr = NULL;
  int next = 0;
  memcpy(&next, b.data+b.maxData*2*sizeof(char *), sizeof(int));
  if (b.curData == b.maxData && next != -1) {
    memcpy(&bptr, b.data+sizeof(char *)*2*b.maxData, sizeof(char *));
    printCurrent(*bptr, str);
  }
  else if (next == -1 && str != NULL) {
    return -1;
  }
  else {
    return 0;
  }
}

/*To free all the buckets, we must start from the end of the chain like a list!*/
/*Then each bucket has its own room allocated*/
void freeBucket(bucket b) {
  avlNode * tree = NULL;

  int next = 0;
  memcpy(&next, b.data+b.maxData*2*sizeof(char *), sizeof(int));
  if (next == -1) {
    for (int i = 0; i < b.curData; ++i) {
      memcpy(&tree, b.data+i*2*sizeof(char *)+sizeof(char *), sizeof(char *));
      avlFree(tree);
    }
    free(b.data);
    return;
  }

  bucket * bptr = NULL;
  memcpy(&bptr, b.data+sizeof(char*)*2*b.maxData, sizeof(char *));
  freeBucket(*bptr);
  for (int i = 0; i < b.curData; ++i) {
    memcpy(&tree, b.data+i*2*sizeof(char *)+sizeof(char *), sizeof(char *));
    avlFree(tree);
  }
  free(b.data);
  free(bptr);
}

/*Free each bucket and then the table*/
void freeHashTable(hashTable * t) {
  for (int i = 0; i < t->size; ++i)
    freeBucket(t->table[i]);

  free(t->table);
  free(t);
}
