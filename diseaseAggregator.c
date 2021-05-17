#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <semaphore.h>
#include "param.h"
#include "functions.h"
#include "aggregator_fun.h"

char flag1 = 0;
char flag2 = 0;
int counter = 0;

void handler (int sig) {
	switch (sig) {
    case SIGCHLD:
			++counter;
      break;
    case SIGINT:
      flag1 = 1;
      break;
    case SIGQUIT:
      flag1 = 1;
      break;
		case SIGPIPE:
			break;
  }
}

int main(int argc, char * argv[]) {

  struct sigaction act = {0};
  act.sa_handler = handler;
  sigfillset(&act.sa_mask);
	// sigaction(SIGINT, &act, NULL);
	sigaction(SIGCHLD, &act, NULL);
	sigaction(SIGPIPE, &act, NULL);
	// sigaction(SIGQUIT, &act, NULL);


  int numOfWorkers = 0;
  char bufferSize[20] = {0};
  char input[20] = {0};
	char serverIP[36] = {0};
	char serverPort[20] = {0};
  Param(argv, input, &numOfWorkers, bufferSize, serverPort, serverIP);

  char path[64] = {0};
  strcpy(path, "./");
  strcpy(path+strlen(path), input);
  /*. and .. is there too!*/
  int countriesNum = dir_counter(path) -2;
  if (countriesNum < numOfWorkers)
    numOfWorkers = countriesNum;
  int countriesPerWorker = countriesNum/numOfWorkers;
  int more = countriesNum%numOfWorkers;

  /*fds[i][0] for READ/fds[i][1] for WRITE*/
  int * fds = malloc(sizeof(int)*numOfWorkers);

  // char readPipe[64] = {0};
  char writePipe[64] = {0};

  int countryLow = 1;
  int countryHigh = countriesPerWorker;
  char buffer[4096] = {0};
  int * pids = malloc(sizeof(int)*numOfWorkers);

  for (int i = 0; i < numOfWorkers; ++i) {
    // strcpy(readPipe, "./pipes/rPipe_");
    strcpy(writePipe, "./pipes/wPipe_");
    // sprintf(readPipe+strlen(readPipe), "%d", i);
    sprintf(writePipe+strlen(writePipe), "%d", i);

    /*if (mkfifo(readPipe, 0666) < 0) {
      perror("Failure at creating pipe");
			int * stats = calloc(3, sizeof(int));
      ext(numOfWorkers, fds, pids, stats);
    }*/
    if (mkfifo(writePipe, 0666) < 0) {
      perror("Failure at creating pipe");
			int * stats = calloc(3, sizeof(int));
      ext(numOfWorkers, fds, pids, stats);
    }

    char workerIndex[5] = {0};
    sprintf(workerIndex, "%d", i);
    if (!(pids[i] = fork()))
      if (execlp("./worker", "./worker", workerIndex, bufferSize, (char *)NULL) < 0) {
        perror("EXEC ERROR");
				int * stats = calloc(3, sizeof(int));
        ext(numOfWorkers, fds, pids, stats);
      }

    /*if ((fds[i][0] = open(readPipe, O_RDONLY | O_NONBLOCK)) < 0) {
      perror("Error opening rpipe1");
			int * stats = calloc(3, sizeof(int));
      ext(numOfWorkers, fds, pids, stats);
    }*/

    while ((fds[i] = open(writePipe, O_WRONLY | O_NONBLOCK | O_TRUNC)) < 0);

    if (more > 0) {
      countryHigh++;
      --more;
    }

    int len = (int)strlen(path);
    memcpy(buffer, &len, sizeof(int));
    memcpy(buffer+sizeof(int), path, len);
    memcpy(buffer+sizeof(int)+len, &countryLow, sizeof(int));
    memcpy(buffer+2*sizeof(int)+len, &countryHigh, sizeof(int));

    write(fds[i], buffer, 3*sizeof(int)+len);

    countryLow = countryHigh+1;
    countryHigh += countriesPerWorker;
  }
	int len1 = strlen(serverIP)+1;
	int len2 = strlen(serverPort)+1;
	for (int i = 0; i < numOfWorkers; ++i) {
		write(fds[i], &len1, sizeof(int));
		write(fds[i], serverIP, len1);
		write(fds[i], &len2, sizeof(int));
		write(fds[i], serverPort, len2);
	}
  int buffSize = atoi(bufferSize);
	masterWaiting(buffSize, pids, numOfWorkers, pids, serverIP, serverPort);
  // instructions(buffer, numOfWorkers, fds, buffSize, pids);
}
