#include <stdlib.h>
#include <stdio.h>
#include "stack.h"

stack * s_init(int n) {
  stack * s = malloc(sizeof(stack));
  s->array = malloc(sizeof(int)*n);
  s->max = n;
  s->top = -1;
  return s;
}

int s_isFull(stack * s) {
  if (s->top == s->max-1)
    return 1;
  return 0;
}

int s_isEmpty(stack * s) {
  if (s->top == -1)
    return 1;
  return 0;
}

int s_push(stack * s, int item) {
  if (s_isFull(s))
    return -1;
  s->top++;
  s->array[s->top] = item;
  return 0;
}

int s_pop(stack * s) {
  if (s_isEmpty(s))
    return -1;
  int item = s->array[s->top];
  s->top--;
  return item;
}

int peek(stack * s) {
  if (s_isEmpty(s))
    return -1;
  int item = s->array[s->top];
  return item;
}
