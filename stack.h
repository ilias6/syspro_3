#ifndef STACK
#define STACK

#include <stdlib.h>

typedef struct stack {
  int * array;
  int max;
  int top;
} stack;

stack * s_init(int);
int s_isFull(stack *);
int s_isEmpty(stack *);
int s_push(stack *, int);
int s_pop(stack *);
int s_peek(stack *);

#endif
