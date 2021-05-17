#ifndef LIST
#define LIST

#include <stdlib.h>
#include "record.h"

typedef Record Item;

typedef struct listNode {
  Item item;
  struct listNode * next;
} listNode;

Item * findRecord(listNode *, char *);
int makeRecord(listNode **, Item *, char *, char *, char *);
int listInsertExit(listNode *, char *, Date);
Item * listInsert(listNode **, Item);
void printList(listNode *);
void listFree(listNode *);

#endif
