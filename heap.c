#include <string.h>
#include "heap.h"
/*Heapify only after deletion,
  since when inseting the tree is mainted sorted*/
/*Top-Bottom heapify*/
void heapify(heapNode * n) {
  if (n == NULL)
    return;

  if ((n->L == NULL) && (n->R == NULL))
    return;

  /*If there is only a left child*/
  if (n->R == NULL) {
    if (n->L->key > n->key)
      heapSwap(n, n->L);
    return;
  }

  if (n->L->key > n->key)
    heapSwap(n->L, n);
  if (n->R->key > n->key)
    heapSwap(n->R, n);

  heapify(n->L);
  heapify(n->R);

}
/*Used when deleted to put the last node on top*/
heapNode * lastNode(heapNode * h) {
  /* Found! */
  if ((h->L == NULL) && (h->R == NULL))
    return h;
  /*Also found !*/
  /*This case is similar to the 3rd in heapify*/
  if ((findMaxHeight(h->L) > findMaxHeight(h->R)) && (h->R == NULL)) {
    heapNode * last = h->L;
    h->L = NULL;
    return last;
  }

  if (findMaxHeight(h->L) > findMaxHeight(h->R)) {
    heapNode * last = h->L;
    /*If the node is going on top, lose connection*/
    if ((h->L->L == NULL) && (h->L->R == NULL))
      h->L = NULL;
    return lastNode(last);
  }
  if (findMaxHeight(h->L) <= findMaxHeight(h->R)) {
    heapNode * last = h->R;
    /*If the node is going on top, lose connection*/
    if ((h->R->L == NULL) && (h->R->R == NULL))
        h->R = NULL;
    return lastNode(last);
  }
}

heapNode * heapDelete(heapNode * h) {
  if (h == NULL)
    return NULL;

  if (h->L == NULL && h->R == NULL) {
    /*switch (h->val) {
      case 0:
        printf("0-20: %d%%\n", h->key);
        break;
      case 1:
        printf("21-40: %d%%\n", h->key);
        break;
      case 2:
        printf("41-60: %d%%\n", h->key);
        break;
      case 3:
        printf("60+: %d%%\n", h->key);
        break;
    }*/
    free(h);
    return NULL;
  }
  heapNode * nodeToDel = h;
  /*Put the last on top*/
  heapNode * t = lastNode(h);
  t->L = h->L;
  t->R = h->R;

  h->L = NULL;
  h->R = NULL;
  h = t;

  /*Sort the heap*/
  heapify(h);

  /*switch (nodeToDel->val) {
    case 0:
      printf("0-20: %d%%\n", nodeToDel->key);
      break;
    case 1:
      printf("21-40: %d%%\n", nodeToDel->key);
      break;
    case 2:
      printf("41-60: %d%%\n", nodeToDel->key);
      break;
    case 3:
      printf("60+: %d%%\n", nodeToDel->key);
      break;
  }*/
  free(nodeToDel);

  return h;
}

/*For finding the position of the last node*/
int findMaxHeight(heapNode *n) {
  if (n == NULL)
    return 0;
  int h1 = findMaxHeight(n->L)+1;
  int h2 = findMaxHeight(n->R)+1;
  if (h2 > h1)
    return h2;
  return h1;
}

/*For finding the position for insertion*/
int findMinHeight(heapNode * n) {
  if (n == NULL)
    return 0;
  int h1 = findMinHeight(n->L)+1;
  int h2 = findMinHeight(n->R)+1;
  if (h2 > h1)
    return h1;
  return h2;
}

heapNode * newNode(Value val, int key) {
  heapNode * newNode;
  newNode = malloc(sizeof(heapNode));
  newNode->L = NULL;
  newNode->R = NULL;
  newNode->key = key;
  newNode->val = val;
  return newNode;
}

void heapSwap(heapNode * n1, heapNode * n2) {
  Value tVal = n1->val;
  int tKey = n1->key;

  n1->key = n2->key;
  n1->val = n2->val;

  n2->key = tKey;
  n2->val = tVal;
}

/*The insert keeps the heap sorted doing its own heapify*/
heapNode * heapInsert(heapNode * h, Value val, int key) {
  if (h == NULL) {
    h = malloc(sizeof(heapNode));
    h->L = NULL;
    h->R = NULL;
    h->key = key;
    h->val = val;
    return h;
  }

  if (findMinHeight(h->L) > findMinHeight(h->R) && h->R == NULL) {
    h->R = newNode(val, key);
    if (h->R->key > h->key)
      heapSwap(h->R, h);
  }
  else if (findMinHeight(h->L) > findMinHeight(h->R)) {
    heapInsert(h->R, val, key);
    if (h->R->key > h->key)
      heapSwap(h->R, h);
  }
  else if (findMinHeight(h->L) <= findMinHeight(h->R) && h->L == NULL) {
    h->L = newNode(val, key);
    if (h->L->key > h->key)
      heapSwap(h->L, h);
  }
  else {
    heapInsert(h->L, val, key);
    if (h->L->key > h->key)
      heapSwap(h->L, h);
  }


  return h;
}

void freeHeap(heapNode * h) {
  if (h == NULL)
    return;
  freeHeap(h->L);
  freeHeap(h->R);
  free(h);
}

/*void printHeap(heapNode * h) {
  if (h == NULL)
    return;
  printf("%d  ", h->key);
  printHeap(h->L);
  printHeap(h->R);
}*/
