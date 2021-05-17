#ifndef HEAP
#define HEAP

#include <stdio.h>
#include <stdlib.h>
#include "record.h"

typedef int Value;
//typedef int Key;

typedef struct heapNode {
  int key;
  Value val;
  struct heapNode * L;
  struct heapNode * R;
} heapNode;

void heapify(heapNode *);
void heapSwap(heapNode *, heapNode *);
int findMaxHeight(heapNode *);
int findMinHeight(heapNode *);
heapNode * lastNode(heapNode *);
heapNode * heapInsert(heapNode *, Value, int);
heapNode * heapDelete(heapNode *);
void freeHeap(heapNode *);
// void printHeap(heapNode *);

#endif
