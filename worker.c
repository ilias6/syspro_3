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
#include "functions.h"
#include "worker_fun.h"
#include "list.h"
#include "hash.h"

#define N_ENTRIES_C 10
#define N_ENTRIES_D 10
#define BUCKET_SIZE 30

char flag1 = 0;
char flag2 = 0;

void handler (int sig) {
	switch (sig) {
    case SIGINT:
      flag1 = 1;
      break;
    case SIGQUIT:
      flag1 = 1;
      break;
		case SIGUSR1:
			flag2 = 1;
			break;
  }
}

typedef struct in_addr in_addr;
typedef struct sockaddr_in sockaddr_in;

int main(int argc, char * argv[]) {

  struct sigaction act = {0};
	sigfillset(&act.sa_mask);
	act.sa_handler = handler;
	// sigaction(SIGINT, &act, NULL);
	// sigaction(SIGQUIT, &act, NULL);
	// sigaction(SIGUSR1, &act, NULL);


  int workerIndex = atoi(argv[1]);
  int bufferSize = atoi(argv[2]);
  char readPipe[64] = {0};
  char writePipe[64] = {0};
  char buffer[8024] = {0};
  char * pipe_buff = malloc(sizeof(bufferSize));
  /*Remember! Worker opens the pipe files in a reversed way*/
  /*fds[0] for write/fds[1] for read*/
  int fd = 0;

  // strcpy(readPipe, "./pipes/rPipe_");
  strcpy(writePipe, "./pipes/wPipe_");
  // sprintf(readPipe+strlen(readPipe), "%d", workerIndex);
  sprintf(writePipe+strlen(writePipe), "%d", workerIndex);


  // while ((fds = open(readPipe, O_WRONLY | O_NONBLOCK | O_TRUNC)) < 0);
  if ((fd = open(writePipe, O_RDONLY | O_NONBLOCK)) < 0) {
    perror("Error opening rpipe2");
    exit(1);
  }

  pipeRead(fd, buffer, pipe_buff, sizeof(int), bufferSize);
  int len = *(int *)buffer;

  pipeRead(fd, buffer, pipe_buff, len, bufferSize);
  char path[64] = {0};
  strcpy(path, buffer);
  pipeRead(fd, buffer, pipe_buff, 2*sizeof(int), bufferSize);
  int low = *(int *)buffer;
  int high = *((int *)buffer+1);
  char ** countries = get_countries(path, low, high);
  int * file_num = malloc(sizeof(int)*(high-low+1));

  hashTable * tC = makeTable(N_ENTRIES_C, BUCKET_SIZE);
  hashTable * tD = makeTable(N_ENTRIES_D, BUCKET_SIZE);
  listNode * list = NULL;
  for (int i = 0; i < (high-low)+1; ++i) {
    strcpy(path+len, countries[i]);
    char ** date_files = get_date_files(path, &file_num[i]);
    sort_date_files(date_files, 0, file_num[i]-1);
    strcpy(path+len+strlen(countries[i]), "/");
    for (int j = 0; j < file_num[i]; ++j) {
      strcpy(path+len+strlen(countries[i])+1, date_files[j]);
      set_up(path, &list, countries[i], date_files[j]);
    }
    for (int j = 0; j < file_num[i]; ++j)
      free(date_files[j]);
    free(date_files);
  }
  listNode * temp = list;
  /*Scan the list and create the hash tables too*/
  while (temp != NULL) {
    hashTableInsert(tD, &temp->item, &temp->item.diseaseID);
    hashTableInsert(tC, &temp->item, &temp->item.country);
    temp = temp->next;
  }

  int n_vir = 0;
  for (int i = 0; i < N_ENTRIES_D; ++i)
    entriesCount(tD->table[i], &n_vir);
  int ** stats = malloc(n_vir*sizeof(int *));
  for (int i = 0; i < n_vir; ++i) {
    stats[i] = malloc(sizeof(int)*4);
    for (int j = 0; j < 4; ++j)
      stats[i][j] = 0;
  }

  char ** viruses = malloc(sizeof(char *)*n_vir);
  for (int i = 0; i < n_vir; ++i)
    viruses[i] = NULL;
  for (int i = 0; i < n_vir; ++i) {
    int index = i;
    for (int j = 0; j < N_ENTRIES_D; ++j) {
      viruses[i] = i_virus(tD->table[j], &index);
      if (viruses[i] != NULL)
        break;
    }
  }
  int bytes = 0;
  char * ptr = buffer+sizeof(int)*2+sizeof(unsigned short)+sizeof(char)+sizeof(struct in_addr);
  char tr[2] = "T";
  char fa[2] = "F";

	// char *** dates = malloc((high-low+1)*sizeof(char **));

  for (int i = 0; i < (high-low+1); ++i) {
    strcpy(path+len, countries[i]);
    file_num[i] = 0;
    char ** date_files = get_date_files(path, &file_num[i]);
		// dates[i] = malloc(file_num[i]*sizeof(char *));
		// for (int f = 0; f < file_num[i]; ++f) {
			// dates[i][f] = malloc(strlen(date_files[f])+1);
			// strcpy(dates[i][f], date_files[f]);
		// }
    sort_date_files(date_files, 0, file_num[i]-1);
    for (int j = 0; j < file_num[i]; ++j) {
      sprintf(ptr+bytes, "%s\n", date_files[j]);
      bytes += (int)strlen(date_files[j])+1;
      sprintf(ptr+bytes, "%s\n", countries[i]);
      bytes += (int)strlen(countries[i])+1;
      Date date = {0};
      stringToDate(date_files[j], &date);
      int count = 0;
      for (int k = 0; k < N_ENTRIES_D; ++k)
        ageStatistics(tD->table[k], date, countries[i], stats, &count);
      for (int v = 0; v < n_vir; ++v) {
        if (!(stats[v][0] == 0 && stats[v][1] == 0 && stats[v][2] == 0 && stats[v][3] == 0)){
          sprintf(ptr+bytes, "%s\n", tr);
          bytes += 2;
          sprintf(ptr+bytes, "%s\n", viruses[v]);
          bytes += (int)strlen(viruses[v])+1;
        }
        else {
          continue;
        }
        sprintf(ptr+bytes, "%d %d %d %d\n", stats[v][0], stats[v][1], stats[v][2], stats[v][3]);
        bytes += 4*sizeof(int)+4;
        stats[v][0] = 0;
        stats[v][1] = 0;
        stats[v][2] = 0;
        stats[v][3] = 0;
      }
      sprintf(ptr+bytes, "%s\n", fa);
      bytes += 2;
    }
    for (int j = 0; j < file_num[i]; ++j)
      free(date_files[j]);
    free(date_files);
  }

	/*tag*/
	bytes += sizeof(int);
	/*port*/
	bytes += sizeof(unsigned short);
	/*ip*/
	bytes += sizeof(struct in_addr);
	int tag = 6;
	char msg = 'W';

	memcpy(buffer, &msg, sizeof(char));
	memcpy(buffer+sizeof(char), &bytes, sizeof(int));
	memcpy(buffer+sizeof(int)+sizeof(char), &tag, sizeof(int));

	char buff[128] = {0};
	pipeRead(fd, buff, pipe_buff, sizeof(int), bufferSize);
	memcpy(&len, buff, sizeof(int));
	pipeRead(fd, buff, pipe_buff, len, bufferSize);
	char * serverIP = malloc(len);
	memcpy(serverIP, buff, len);
	pipeRead(fd, buff, pipe_buff, sizeof(int), bufferSize);
	memcpy(&len, buff, sizeof(int));
	pipeRead(fd, buff, pipe_buff, len, bufferSize);
	char * serverPort = malloc(len);
	memcpy(serverPort, buff, len);
	int serverport = atoi(serverPort);
	free(serverPort);

	sockaddr_in server = {0};
	server.sin_family = AF_INET;
	if (!inet_aton(serverIP, &server.sin_addr)) {
		perror("inet_aton");
		exit(1);
	}

	server.sin_port = htons(serverport);
	int sock_fd = socket(server.sin_family, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		perror("sock fd");
		exit(1);
	}

	if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("connect");
		exit(1);
	}

	int val = 1;
	sockaddr_in query = {0};
	query.sin_family = AF_INET;
	query.sin_addr.s_addr = htonl(INADDR_ANY);
	int querySock = socket(server.sin_family, SOCK_STREAM, 0);
  if (querySock < 0) {
    perror("sock fd");
    exit(1);
  }
	struct addrinfo hints;
	if (bind(querySock, (struct sockaddr *)&query , sizeof(sockaddr_in)) < 0) {
		perror("bind");
		exit(1);
	}
	if (setsockopt(querySock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int)) < 0) {
		perror("set sock opt");
		exit(1);
	}
  struct addrinfo * result;
	char addrstr[100] = {0};
	memset(&hints, 0, sizeof(struct addrinfo));
	socklen_t s_len = sizeof(sockaddr_in);
	getsockname(querySock, (struct sockaddr *)&query, &s_len);
	/*char strport[20] = {0};
	sprintf(strport, "%d", query.sin_port);
	val = getaddrinfo(NULL, strport, &hints, &result);
	if (val != 0) {
		perror("getaddrinfo");
		exit(1);
	}
	sockaddr_in * new = (sockaddr_in *)result->ai_addr;
	printf("%d %d %d %d\n",query.sin_port, new->sin_port, query.sin_addr.s_addr, new->sin_addr.s_addr);*/
	memcpy(buffer+sizeof(char)+2*sizeof(int), &query.sin_port, sizeof(unsigned short));
	memcpy(buffer+sizeof(char)+2*sizeof(int)+sizeof(unsigned short), &query.sin_addr, sizeof(struct in_addr));

	if (listen(querySock, 600) < 0) {
    perror("listen");
    exit(1);
  }

	safeWrite(sock_fd, buffer, sizeof(int)+sizeof(char)+bytes);
	close(sock_fd);

  worker_space * ws = malloc(sizeof(worker_space));
  ws->total = 0;
  ws->fail = 0;
  ws->success = 0;
  ws->n_vir = n_vir;
  ws->n_countries = high-low+1;
  ws->list = list;
  ws->tC = tC;
  ws->tD = tD;
  ws->countries = countries;
  ws->file_num = file_num;
  ws->stats = stats;
  ws->pipe_buff = pipe_buff;
  ws->viruses = viruses;
  ws->fd = fd;
  ws->bufferSize = bufferSize;
	ws->serverIP = serverIP;
	ws->serverPort = serverport;

	fd_set rfds;
  int retval = 0;
  struct timeval tv = {0};
  tv.tv_usec = 100;
  int newSock = 0;

	while (1) {
		if (flag1)
			worker_exit(ws);
		FD_ZERO(&rfds);
    FD_SET(querySock, &rfds);
    int retval = select(querySock+1, &rfds, NULL, NULL, &tv);
    if (retval > 0) {
      if ((newSock = accept(querySock, NULL, NULL)) < 0) {
        perror("accept");
        exit(1);
      }
			for (int i = 0; i < 8024; ++i)
				buffer[i] = 0;
			int size = 0;
			pipeRead(newSock, buffer, ws->pipe_buff, sizeof(int), ws->bufferSize);
			memcpy(&size, buffer, sizeof(int));
			pipeRead(newSock, buffer, ws->pipe_buff, size, ws->bufferSize);

			ws->ans_sock = newSock;
			instruction_parse(buffer, ws, size, newSock);
			usleep(tv.tv_usec);
			close(newSock);
		}
	}

  return (0);
}
