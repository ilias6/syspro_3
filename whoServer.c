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
#include "whoServer.h"
#include "functions.h"
#include "stack.h"

typedef struct sockaddr_in sockaddr_in;

char flag1 = 0;

void handler (int sig) {
	switch (sig) {
    case SIGINT:
      flag1 = 1;
      break;
    case SIGQUIT:
      flag1 = 1;
      break;
    }
  }

int main(int argc, char * argv[]) {
  struct sigaction act = {0};
  act.sa_handler = handler;
  sigfillset(&act.sa_mask);
	// sigaction(SIGINT, &act, NULL);
  // sigaction(SIGQUIT, &act, NULL);

  int queryPortNum = 0;
  int statisticsPortNum = 0;
  int numThreads = 0;
  int bufferSize = 0;
  Param(argv, &queryPortNum, &statisticsPortNum, &numThreads, &bufferSize);
  pthread_mutex_t mutex, mutex2;
  if (pthread_mutex_init(&mutex, NULL) != 0) {
    perror("mutex");
    exit(1);
  }
	if (pthread_mutex_init(&mutex2, NULL) != 0) {
		perror("mutex");
		exit(1);
	}
  sem_t serv_sem;
  if (sem_init(&serv_sem, 0, 0) < 0) {
    perror("serv_sem");
    exit(1);
  }
  sem_t thr_sem;
  if (sem_init(&thr_sem, 0, 0) < 0) {
    perror("thr_sem");
    exit(1);
  }
  pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
  stack * s = s_init(bufferSize);
  server_tools * tools = calloc(1, sizeof(server_tools));
  unsigned short * ports = calloc(1, sizeof(unsigned short));
  unsigned long * ips = calloc(1, sizeof(unsigned long));

  tools->s = s;
  tools->cond = &cond;
  tools->mutex = &mutex;
	tools->mutex2 = &mutex2;
  tools->serv_sem = &serv_sem;
  tools->thr_sem = &thr_sem;
  tools->workers_num = 1;
  tools->ips = ips;
  tools->ports = ports;

  pthread_t * t_id = malloc(sizeof(pthread_t)*numThreads);
  for (int i = 0; i < numThreads; ++i)
    pthread_create(&t_id[i], NULL, threadFun, tools);

  sockaddr_in server = {0};
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(statisticsPortNum);

  int statisticsSock = socket(server.sin_family, SOCK_STREAM, 0);
  if (statisticsSock < 0) {
    perror("sock fd");
    exit(1);
  }
  int val = 1;

  if (setsockopt(statisticsSock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int)) < 0) {
    perror("set sock opt");
    exit(1);
  }
  if (bind(statisticsSock, (struct sockaddr *)&server, sizeof(sockaddr_in)) < 0) {
    perror("bind");
    exit(1);
  }
  if (listen(statisticsSock, 50) < 0)  {
    perror("listen");
    exit(1);
  }

  server.sin_port = htons(queryPortNum);

  int querySock = socket(server.sin_family, SOCK_STREAM, 0);
  if (querySock < 0) {
    perror("sock fd");
    exit(1);
  }

  if (setsockopt(querySock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int)) < 0) {
    perror("set sock opt");
    exit(1);
  }

  if (bind(querySock, (struct sockaddr *)&server , sizeof(sockaddr_in)) < 0) {
    perror("bind");
    exit(1);
  }
  if (listen(querySock, 500) < 0)  {
    perror("listen");
    exit(1);
  }

  sockaddr_in client = {0};
  socklen_t clientlen = 0;


  fd_set rfds;
  int retval = 0;
  struct timeval tv = {0};
  tv.tv_usec = 10;

  int newSock = 0;
  while (1) {
    if (flag1)
      proper_exit(tools, t_id, numThreads);
    FD_ZERO(&rfds);
    FD_SET(statisticsSock, &rfds);
    retval = select(statisticsSock+1, &rfds, NULL, NULL, &tv);
    if (retval > 0) {
      if (flag1)
        proper_exit(tools, t_id, numThreads);
      if ((newSock = accept(statisticsSock, (struct sockaddr *)&client, &clientlen)) < 0) {
        perror("accept");
        exit(1);
      }
      if (flag1)
        proper_exit(tools, t_id, numThreads);
      pthread_mutex_lock(&mutex);
      s_push(s, newSock);
      if (flag1)
        proper_exit(tools, t_id, numThreads);
      if (s->top == 0)
        pthread_cond_broadcast(&cond);
      // printf("Accepted connection\n");
      pthread_mutex_unlock(&mutex);
    }
    if (flag1)
      proper_exit(tools, t_id, numThreads);
    FD_ZERO(&rfds);
    FD_SET(querySock, &rfds);
    retval = select(querySock+1, &rfds, NULL, NULL, &tv);
    if (flag1)
      proper_exit(tools, t_id, numThreads);
    if (retval > 0) {
      if (flag1)
        proper_exit(tools, t_id, numThreads);
      if ((newSock = accept(querySock, (struct sockaddr *)&client, &clientlen)) < 0) {
        perror("accept");
        exit(1);
      }
      pthread_mutex_lock(&mutex);
      if (flag1)
        proper_exit(tools, t_id, numThreads);
      s_push(s, newSock);
      if (s->top == 0)
        pthread_cond_broadcast(&cond);
      if (flag1)
        proper_exit(tools, t_id, numThreads);
      printf("Accepted connection\n");
      pthread_mutex_unlock(&mutex);
    }
    if (flag1)
      proper_exit(tools, t_id, numThreads);
    if (s_isFull(s))
      sem_wait(&serv_sem);

  }
  for (int i = 0; i < numThreads; ++i)
    pthread_join(t_id[i], NULL);

}
