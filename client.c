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
#include "client.h"
#include "functions.h"
#include "stack.h"

typedef struct sockaddr_in sockaddr_in;

int counterr = 0;
int counterr1 = 0;

int main(int argc, char * argv[]) {
  char queryFile[20] = {0};
  int serverPort = 0;
  int numThreads = 0;
  char serverIP[32] = {0};
  Param(argv, queryFile, &serverPort, &numThreads, serverIP);

  FILE * f = fopen(queryFile, "r");
  pthread_t * t_id = malloc(sizeof(pthread_t)*numThreads);
  char buffer[100] = {0};
  client_tools * tools = malloc(sizeof(client_tools)*numThreads);

  sem_t thr_sem;
  if (sem_init(&thr_sem, 0, 0) < 0) {
    perror("serv_sem");
    exit(1);
  }
  for (int i = 0; i < numThreads; ++i) {
    tools[i].server_port = serverPort;
    tools[i].server_IP = serverIP;
    tools[i].thr_sem = &thr_sem;
    if (fgets(buffer, 100, f) == NULL)
      break;
    int len = strlen(buffer)+1;
    /*Every thread has its own request*/
    tools[i].instr = malloc(len);
    strncpy(tools[i].instr, buffer, len);
    pthread_create(&t_id[i], NULL, threadFun, &tools[i]);
  }
  fclose(f);
  for (int i = 0; i < numThreads; ++i)
    sem_post(&thr_sem);

  for (int i = 0; i < numThreads; ++i)
    pthread_join(t_id[i], NULL);

  sem_destroy(&thr_sem);
  for (int i = 0; i < numThreads; i++)
    free_tools(&tools[i]);
  free(tools);
  free(t_id);
  /*Tell server to quit*/
  finished(serverIP, serverPort);
  return 0;
}
