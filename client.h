#ifndef CLIENT
#define CLIENT

#include <stdlib.h>
#include <semaphore.h>


typedef struct client_tools {
  char * instr;
  int server_port;
  char * server_IP;
  sem_t * thr_sem;
  int * ports;
  int ports_num;
} client_tools;

void Param(char ** argv, char *, int *, int *, char *);
void get_answer(int, char *, char *, int, char *);
void finished(char *, int);
void * threadFun(void *);
void free_tools(client_tools *);


#endif
