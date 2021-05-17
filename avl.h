#ifndef AVL
#define AVL

#include "list.h"
#include "record.h"
#include <stdlib.h>

typedef Date Key;

typedef struct avlNode {
  Record * r;
  Key key;
  int H;
  struct avlNode * L;
  struct avlNode * R;
} avlNode;

int max(int, int);
int getHeight(avlNode *);
int getBalance(avlNode *);
int avlSearch(avlNode *, Date, Date, char *, char *);
void ageSearch(avlNode *, Date, char *, char *, int *);
int countAdmissions(avlNode *, char *, Date, Date);
int countDischarges(avlNode *, char *, Date, Date);
void topkAgeSearch(avlNode *, char *, Date, Date, int *);
avlNode * rightRot(avlNode *);
avlNode * leftRot(avlNode *);
avlNode * avlNewNode(Record *);
avlNode * avlInsert(avlNode *, Record *);
void avlInOrder(avlNode *);
void avlFree(avlNode *);

#endif
