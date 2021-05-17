#ifndef WHOSERVER
#define WHOSERVER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <errno.h>
#include <pthread.h>
#include "stack.h"

typedef struct sockaddr_in sockaddr_in;

typedef struct server_tools {
  stack * s;
  pthread_mutex_t * mutex;
  pthread_mutex_t * mutex2;
  sem_t * serv_sem;
  sem_t * thr_sem;
  pthread_cond_t * cond;
  unsigned short * ports;
  unsigned long * ips;
  int workers_num;
} server_tools;

void Param(char ** argv, int *, int *, int *, int *);
int ageStats(int , int);
void get_msg(int, int, char *, int *, unsigned short **, unsigned long **, int *, server_tools *);
int get_question(int, int, char *);
int remake_answer(char *, int *, char *, char *, int, server_tools *);
void get_answer(int, int, char *, int *, unsigned short **, unsigned long **, int *, server_tools *);
void * threadFun(void *);
void proper_exit(server_tools *, pthread_t *, int);



#endif
