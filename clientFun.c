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
#include "functions.h"
#include "client.h"
#include "functions.h"
#include "stack.h"

typedef struct sockaddr_in sockaddr_in;

void Param(char ** argv, char * queryFile, int * serverPort, int * numThreads, char * serverIP) {

  char flags[4] = {0};

  for (int i = 1; i < 9; i += 2) {
    if (!flags[0] && !strcmp(argv[i], "-q")) {
      flags[0] = 1;
      strcpy(queryFile, argv[i+1]);
    }
    else if (!flags[1] && !strcmp(argv[i], "-w")) {
      flags[1] = 1;
      *numThreads = atoi(argv[i+1]);
    }
    else if (!flags[2] && !strcmp(argv[i], "-sip")) {
      flags[2] = 1;
      strcpy(serverIP, argv[i+1]);
    }
    else if (!flags[3] && !strcmp(argv[i], "-sp")) {
      flags[3] = 1;
      *serverPort = atoi(argv[i+1]);
    }
    else {
      printf("To run properly:\n./diseaseAggregator -i input_dir");
      printf("–w numberOfWorkers -b bufferSize -s serverIP -p serverPort\n");
      exit(1);
    }
  }
  if ( !flags[0] | !flags[1] | !flags[2] | !flags[3]) {
    printf("To run properly:\n./diseaseAggregator -i input_dir");
    printf("–w numberOfWorkers -b bufferSize -s serverIP -p serverPort\n");
    exit(1);
  }
}

void finished(char * sip, int sp) {
  sockaddr_in server = {0};
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(sp);

	if (!inet_aton(sip, &server.sin_addr)) {
		perror("inet_aton");
		exit(1);
	}

	int sock_fd = socket(server.sin_family, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		perror("sock fd");
		exit(1);
	}

	if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("connect");
		exit(1);
	}
  char msg = 'F';
  safeWrite(sock_fd, &msg, sizeof(char));
  close(sock_fd);
}

void * threadFun(void * args) {
  client_tools * tools = args;
  char * instr = tools->instr;
  int serverPort = tools->server_port;
  char * serverIP = tools->server_IP;
  sem_wait(tools->thr_sem);

  sockaddr_in server = {0};
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(serverPort);

	if (!inet_aton(serverIP, &server.sin_addr)) {
		perror("inet_aton");
		exit(1);
	}

	int sock_fd = socket(server.sin_family, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		perror("sock fd");
		exit(1);
	}
  
	if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("connect");
		exit(1);
	}

  int size = lineSize(instr);
  char msg = 'C';
  safeWrite(sock_fd, &msg, sizeof(char));
  safeWrite(sock_fd, &size, sizeof(int));
  safeWrite(sock_fd, instr, size);
  char buffer[4096] = {0};
  char * pipe_buff = malloc(10);
  fd_set rfds;
  int retval = 0;
  struct timeval tv = {0};
  tv.tv_usec = 1000;
  /*Ensure that server got the answer*/
  // sleep(20);
  long timeout = 0;
  // while (1) {
  FD_ZERO(&rfds);
  FD_SET(sock_fd, &rfds);
  while (select(sock_fd+1, &rfds, NULL, NULL, &tv));
    // retval = select(sock_fd+1, &rfds, NULL, NULL, &tv);
    // if (retval > 0) {
      get_answer(sock_fd, buffer, pipe_buff, 10, instr);
      close(sock_fd);
    // }
  // }
    // ++timeout;
    // if (timeout == 10)
      // break;
  // }

  free(pipe_buff);
}

void get_answer(int fd, char * buffer, char * pipe_buff, int bufferSize, char * instr) {
  pipeRead(fd, buffer, pipe_buff, sizeof(int), bufferSize);
  int size = 0;
  memcpy(&size, buffer, sizeof(int));
  int lSize = lineSize(instr)+1;
  pipeRead(fd, buffer+lSize, pipe_buff, size, bufferSize);
  memcpy(buffer, instr, lSize);
  if (size == sizeof(int))
    printf("%s%d\n", instr, *(int *)(buffer+lSize));
  else
    safeWrite(0, buffer, size+lSize);
}

void free_tools(client_tools * t) {
  free(t->instr);
}
