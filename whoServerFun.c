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
#include "whoServer.h"
#include "functions.h"
#include "stack.h"

typedef struct sockaddr_in sockaddr_in;

int counter1 = 0;
int counter2 = 0;

void proper_exit(server_tools * tools, pthread_t * t_id, int numThreads) {
  free(t_id);
  while (pthread_mutex_unlock(tools->mutex));
  pthread_mutex_destroy(tools->mutex);
  sem_destroy(tools->thr_sem);
  sem_destroy(tools->thr_sem);
  free(tools->ports)  ;
  free(tools->ips);
  pthread_cond_destroy(tools->cond);
  exit(1);
}

void Param(char ** argv, int * queryPortNum, int * statisticsPortNum, int * numThreads, int * bufferSize) {

  char flags[4] = {0};

  for (int i = 1; i < 9; i += 2) {
    if (!flags[0] && !strcmp(argv[i], "-q")) {
      flags[0] = 1;
      *queryPortNum = atoi(argv[i+1]);
    }
    else if (!flags[1] && !strcmp(argv[i], "-w")) {
      flags[1] = 1;
      *numThreads = atoi(argv[i+1]);
    }
    else if (!flags[2] && !strcmp(argv[i], "-b")) {
      flags[2] = 1;
      *bufferSize = atoi(argv[i+1]);
    }
    else if (!flags[3] && !strcmp(argv[i], "-s")) {
      flags[3] = 1;
      *statisticsPortNum = atoi(argv[i+1]);
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


void * threadFun(void * args) {
  server_tools * tools = args;
  pthread_mutex_t * mutex = tools->mutex;
  sem_t * serv_sem = tools->serv_sem;
  sem_t * thr_sem = tools->thr_sem;
  pthread_cond_t * cond = tools->cond;
  stack * buffer = tools->s;
  unsigned short ** ports = &tools->ports;
  unsigned long ** ips = &tools->ips;
  int * workers_num = &tools->workers_num;

  int sock = 0;
  while (1) {
    pthread_mutex_lock(mutex);
    while (s_isEmpty(buffer))
      pthread_cond_wait(cond, mutex);

    int sock = s_pop(buffer);
    if (buffer->top == buffer->max-2)
      sem_post(serv_sem);

    pthread_mutex_unlock(mutex);
    get_msg(sock, 1, NULL, NULL, ports, ips, workers_num, tools);
    close(sock);
  }
}

void get_msg(int fd, int buffSize, char * flag, int * count, unsigned short ** ports, unsigned long ** ips, int * workers_num, server_tools * tools) {
  char buffer[512] = {0};
  char req[256] = {0};
  char * pipe_buff = malloc(sizeof(buffSize));
  pipeRead(fd, buffer, pipe_buff, sizeof(char), buffSize);
  char msg = *(char *)buffer;
  if (msg == 'W') {
    get_answer(fd, buffSize, NULL, NULL, ports, ips, workers_num ,tools);
  }
  else if (msg == 'C') {
    int len = get_question(fd, buffSize, req);
    sockaddr_in worker = {0};
    worker.sin_family = AF_INET;
    char buff[4096] = {0};
    int cur_bytes = 0;
    int count = 0;
    char flag = 0;
    pthread_mutex_lock(tools->mutex2);
    ++counter1;
    if (!counter1%50)
      printf("\n\n\n\n\n\n\n\t\t%d\n\n\n\n\n\n\n\n\n", counter1);
    pthread_mutex_unlock(tools->mutex2);

    for (int i = 0; i < *workers_num-1; ++i) {
      memcpy(&worker.sin_addr, &(*ips)[i], sizeof(struct in_addr));
      memcpy(&worker.sin_port, &(*ports)[i], sizeof(unsigned short));
      // printf("------------------------\n");
  	  int worker_fd = socket(worker.sin_family, SOCK_STREAM, 0);
  	  if (worker_fd < 0) {
  		  perror("sock fd");
  		  exit(1);
  	  }
  	  if (connect(worker_fd, (struct sockaddr *)&worker, sizeof(worker)) < 0) {
  		  perror("connect worker");
  		  continue;
  	  }
      // pthread_mutex_lock(tools->mutex2);
      // printf("CONNECTED %d\n", i);
      safeWrite(worker_fd, &len, sizeof(int));
      safeWrite(worker_fd, req, len);
      // printf("WAITING\n");
      pipeRead(worker_fd, buffer, pipe_buff, sizeof(int), buffSize);
      // pthread_mutex_unlock(tools->mutex2);
      // printf("GOT ANS\n");
      int size = 0;
      memcpy(&size, buffer, sizeof(int));
      pipeRead(worker_fd, buffer, pipe_buff, size, buffSize);
      int bytes = remake_answer(buffer, &count, buff+cur_bytes, &flag, size, tools);
      cur_bytes += bytes;
      close(worker_fd);
    }
    pthread_mutex_lock(tools->mutex2);
    ++counter2;
    if (!counter1%50)
      printf("\n\n\n\n\n\n\n\t\t\%d\n\n\n\n\n\n\n\n\\n", counter2);
    pthread_mutex_unlock(tools->mutex2);
    // printf("%d\n", cur_bytes);
    if (!flag) {
      // safeWrite(0, buff, cur_bytes);
      safeWrite(fd, &cur_bytes, sizeof(int));
      safeWrite(fd, buff, cur_bytes);
    }
    else {
      // printf("%d\n", count);
      int size = sizeof(int);
      safeWrite(fd, &size, sizeof(int));
      safeWrite(fd, &count, sizeof(int));
    }
  }
  else if (msg == 'F') {
    close(fd);
    free(pipe_buff);
    pipe_buff = NULL;
    exit(1);
  }
  else {
    printf("Error: Unknown message!\n");
  }
  close(fd);
  if (pipe_buff != NULL)
    free(pipe_buff);
}

int get_question(int fd, int buffSize, char * buffer) {
  char * pipe_buff = malloc(sizeof(buffSize));
  int size = 0;
  pipeRead(fd, buffer, pipe_buff, sizeof(int), buffSize);
  memcpy(&size, buffer, sizeof(int));
  pipeRead(fd, buffer, pipe_buff, size, buffSize);
  free(pipe_buff);
  return size;
}

int remake_answer(char * buffer, int * count, char * ans, char * flag, int bytes, server_tools * tools) {
  int retval = 0;
  int tag = *(int *)buffer;
  char * ptr = buffer;
  ptr += sizeof(int);
  bytes -= sizeof(int);
  int pid = 0;
  // pthread_mutex_lock(tools->mutex);
  switch (tag) {
    case 1:
      if (bytes == 6) {
        break;
      }
      while (bytes > 0) {
        int val = 0;
        int key = 0;
        memcpy(&val, ptr, sizeof(int));
        memcpy(&key, ptr+sizeof(int), sizeof(int));
        ptr += 2*sizeof(int);
        bytes -= 2*sizeof(int);
        switch (val) {
          case 0:
            sprintf(ans+retval, "0-20: %d%%\n", key);
            retval += 9+sizeof(int);
            // printf("0-20: %d%%\n", key);
            break;
          case 1:
            sprintf(ans+retval, "21-40: %d%%\n", key);
            retval += 10+sizeof(int);
            // printf("21-40: %d%%\n", key);
            break;
          case 2:
            sprintf(ans+retval, "41-60: %d%%\n", key);
            retval += 10+sizeof(int);
            // printf("41-60: %d%%\n", key);
            break;
          case 3:
            sprintf(ans+retval, "60+: %d%%\n", key);
            retval += 8+sizeof(int);
            // printf("60+: %d%%\n", key);
            break;
        }
      }
      break;
    case 2:
      *flag = 1;
      int n = 0;
      memcpy(&n, ptr, sizeof(int));
      (*count) += n;
      break;
    case 3:
      if (bytes == 6) {
        break;
      }
      char id[32] = {0};
      char fname[32] = {0};
      char lname[32] = {0};
      char virus[32] = {0};
      char country[32] = {0};
      int age = 0;
      Date d1 = {0};
      Date d2 = {0};
      int len = 0;

      memcpy(&len, ptr, sizeof(int));
      ptr += sizeof(int);
      memcpy(&id, ptr, len);
      ptr += len;

      memcpy(&len, ptr, sizeof(int));
      ptr += sizeof(int);
      memcpy(&fname, ptr, len);
      ptr += len;

      memcpy(&len, ptr, sizeof(int));
      ptr += sizeof(int);
      memcpy(&lname, ptr, len);
      ptr += len;

      memcpy(&len, ptr, sizeof(int));
      ptr += sizeof(int);
      memcpy(&virus, ptr, len);
      ptr += len;

      memcpy(&len, ptr, sizeof(int));
      ptr += sizeof(int);
      memcpy(&country, ptr, len);
      ptr += len;

      memcpy(&age, ptr, sizeof(int));
      ptr += sizeof(int);
      memcpy(&d1, ptr, sizeof(Date));
      ptr += sizeof(Date);
      memcpy(&d2, ptr, sizeof(Date));
      // printf("ID: %s\n", id);
      sprintf(ans+retval, "ID: %s\n", id);
      retval += strlen(id)+1 +6;
      // printf("First name: %s\n", fname);
      sprintf(ans+retval, "First name: %s\n", fname);
      retval += strlen(fname)+1 +14;
      // printf("Last name: %s\n", lname);
      sprintf(ans+retval, "Last name: %s\n", lname);
      retval += strlen(lname)+1 +13;
      // printf("Disease: %s\n", virus);
      sprintf(ans+retval, "Disease: %s\n", virus);
      retval += strlen(virus)+1 +11;
      // printf("Country: %s\n", country);
      sprintf(ans+retval, "Country: %s\n", country);
      retval += strlen(country)+1 +10;
      // printf("Age: %d\n", age);
      sprintf(ans+retval, "Age: %d\n", age);
      retval += sizeof(int) +7;
      // printf("Entry date: ");
      // printDate(d1);
      sprintf(ans+retval, "Entry date: ");
      retval += 12;
      printDatetoAns(d1, ans+retval, &retval);
      // printf("Exit date: ");
      // printDate(d2);
      sprintf(ans+retval, "Exit date: ");
      retval += 11;
      printDatetoAns(d2, ans+retval, &retval);
      break;
    case 4:
      if (bytes == 6) {
        break;
      }
      while (bytes > 0) {
        char country[32] = {0};
        int len = 0;
        int count = 0;
        memcpy(&len, ptr, sizeof(int));
        ptr += sizeof(int);
        bytes -= sizeof(int);
        memcpy(country, ptr, len);
        ptr += len;
        bytes -= len;
        memcpy(&count, ptr, sizeof(int));
        ptr += sizeof(int);
        bytes -= sizeof(int);
        // printf("%s %d\n", country, count);
        sprintf(ans+retval, "%s %d\n", country, count);
        retval += strlen(country)+4 +sizeof(int);
        // printf("%d\n", retval);
      }
      break;
    case 5:
      if (bytes == 6) {
        break;
      }
      while (bytes > 0) {
        char country[32] = {0};
        int len = 0;
        int count = 0;
        memcpy(&len, ptr, sizeof(int));
        ptr += sizeof(int);
        bytes -= sizeof(int);
        memcpy(country, ptr, len);
        ptr += len;
        bytes -= len;
        memcpy(&count, ptr, sizeof(int));
        ptr += sizeof(int);
        bytes -= sizeof(int);
        // printf("%s %d\n", country, count);
        sprintf(ans+retval, "%s %d\n", country, count);
        retval += strlen(country)+1 +sizeof(int)+3;
      }
      break;
    }
    // pthread_mutex_unlock(tools->mutex);
    return retval;
}

void get_answer(int fd, int buffSize, char * flag, int * count, unsigned short ** ports, unsigned long ** ips, int * workers_num, server_tools * tools) {
  char buffer[4096] = {0};
  char * pipe_buff = malloc(buffSize);
  pipeRead(fd, buffer, pipe_buff, sizeof(int), buffSize);
  int bytes = *(int *)buffer;
  pipeRead(fd, buffer, pipe_buff, bytes, buffSize);
  free(pipe_buff);
  int tag = *(int *)buffer;
  char * ptr = buffer;
  ptr += sizeof(int);
  bytes -= sizeof(int);
  int pid = 0;
  switch (tag) {
    case 0 :
      pid = *(int *)ptr;
      ptr += sizeof(int);
      bytes -= sizeof(int);
      while (bytes > 0) {
        int len = 0;
        char * str = NULL;
        memcpy(&len, ptr, sizeof(int));
        bytes -= sizeof(int);
        ptr += sizeof(int);
        str = malloc(len);
        memcpy(str, ptr, len);
        bytes -= len;
        ptr += len;
        printf("%s %d\n", str, pid);
        free(str);
      }
      break;
    case 1:
      if (bytes == 6) {
        break;
      }
      while (bytes > 0) {
        int val = 0;
        int key = 0;
        memcpy(&val, ptr, sizeof(int));
        memcpy(&key, ptr+sizeof(int), sizeof(int));
        ptr += 2*sizeof(int);
        bytes -= 2*sizeof(int);
        switch (val) {
          case 0:
            printf("0-20: %d%%\n", key);
            break;
          case 1:
            printf("21-40: %d%%\n", key);
            break;
          case 2:
            printf("41-60: %d%%\n", key);
            break;
          case 3:
            printf("60+: %d%%\n", key);
            break;
        }
      }
      break;
    case 2:
      *flag = 1;
      int n = 0;
      memcpy(&n, ptr, sizeof(int));
      (*count) += n;
      break;
    case 3:
      if (bytes == 6) {
        break;
      }
      char id[32] = {0};
      char fname[32] = {0};
      char lname[32] = {0};
      char virus[32] = {0};
      char country[32] = {0};
      int age = 0;
      Date d1 = {0};
      Date d2 = {0};
      int len = 0;

      memcpy(&len, ptr, sizeof(int));
      ptr += sizeof(int);
      memcpy(&id, ptr, len);
      ptr += len;

      memcpy(&len, ptr, sizeof(int));
      ptr += sizeof(int);
      memcpy(&fname, ptr, len);
      ptr += len;

      memcpy(&len, ptr, sizeof(int));
      ptr += sizeof(int);
      memcpy(&lname, ptr, len);
      ptr += len;

      memcpy(&len, ptr, sizeof(int));
      ptr += sizeof(int);
      memcpy(&virus, ptr, len);
      ptr += len;

      memcpy(&len, ptr, sizeof(int));
      ptr += sizeof(int);
      memcpy(&country, ptr, len);
      ptr += len;

      memcpy(&age, ptr, sizeof(int));
      ptr += sizeof(int);
      memcpy(&d1, ptr, sizeof(Date));
      ptr += sizeof(Date);
      memcpy(&d2, ptr, sizeof(Date));
      printf("ID: %s\n", id);
      printf("First name: %s\n", fname);
      printf("Last name: %s\n", lname);
      printf("Disease: %s\n", virus);
      printf("Country: %s\n", country);
      printf("Age: %d\n", age);
      printf("Entry date: ");
      printDate(d1);
      printf("Exit date: ");
      printDate(d2);
      break;
    case 4:
      if (bytes == 6) {
        break;
      }
      while (bytes > 0) {
        char country[32] = {0};
        int len = 0;
        int count = 0;
        memcpy(&len, ptr, sizeof(int));
        ptr += sizeof(int);
        bytes -= sizeof(int);
        memcpy(country, ptr, len);
        ptr += len;
        bytes -= len;
        memcpy(&count, ptr, sizeof(int));
        ptr += sizeof(int);
        bytes -= sizeof(int);
        printf("%s %d\n", country, count);
      }
      break;
    case 5:
      if (bytes == 6) {
        break;
      }
      while (bytes > 0) {
        char country[32] = {0};
        int len = 0;
        int count = 0;
        memcpy(&len, ptr, sizeof(int));
        ptr += sizeof(int);
        bytes -= sizeof(int);
        memcpy(country, ptr, len);
        ptr += len;
        bytes -= len;
        memcpy(&count, ptr, sizeof(int));
        ptr += sizeof(int);
        bytes -= sizeof(int);
        printf("%s %d\n", country, count);
      }
      break;
    case 6:
      pthread_mutex_lock(tools->mutex2);
      (*ports)[(*workers_num)-1] = *(unsigned short *)ptr;
      ptr += sizeof(unsigned short);
      bytes -= sizeof(unsigned short);
      ptr += sizeof(struct in_addr);
      bytes -= sizeof(struct in_addr);
      sockaddr_in addr = {0};
      socklen_t s_len = 1000;
      if (getpeername(fd, (struct sockaddr *)&addr, &s_len) == -1) {
        perror("sdfs");
        exit(1);
      }
      (*ips)[(*workers_num)-1] = addr.sin_addr.s_addr;

      (*workers_num)++;
      *ports = realloc(*ports, (*workers_num)*sizeof(unsigned short));
      *ips = realloc(*ips, (*workers_num)*sizeof(unsigned long));
      pthread_mutex_unlock(tools->mutex2);

      while (bytes > 0) {
        char flag[2] = {0};
        char country[64] = {0};
        char virus[64] = {0};
        char date[64] = {0};
        int stats[4] = {0};
        int counter = 0;
        sscanf(ptr, "%s", date);
        // printf("%s\n", date);
        bytes -= (int)strlen(date)+1;
        ptr += (int)strlen(date)+1;
        sscanf(ptr, "%s", country);
        // printf("%s\n", country);
        bytes -= (int)strlen(country)+1;
        ptr += (int)strlen(country)+1;
        sscanf(ptr, "%s", flag);
        bytes -= 2;
        ptr += 2;
        while (!strcmp(flag, "T")) {
          sscanf(ptr, "%s\n", virus);
          // printf("%s\n", virus);
          bytes -= (int)strlen(virus)+1;
          ptr += (int)strlen(virus)+1;
          sscanf(ptr, "%d %d %d %d\n", &stats[0], &stats[1], &stats[2], &stats[3]);
          // printf("Age range 0-20 years: %d cases\n", stats[0]);
          // printf("Age range 21-40 years: %d cases\n", stats[1]);
          // printf("Age range 41-60 years: %d cases\n", stats[2]);
          // printf("Age range 60+ years: %d cases\n", stats[3]);
          bytes -= 4*sizeof(int)+4;
          ptr += 4*sizeof(int)+4;
          sscanf(ptr, "%s\n", flag);
          bytes -= 2;
          ptr += 2;
        }
      }
      break;
  }
}

int ageStats(int fd, int buffSize) {
  char buffer[8096] = {0};
  char * pipe_buff = malloc(buffSize);
  pipeRead(fd, buffer, pipe_buff, sizeof(int), buffSize);
  int bytes = 0;
  memcpy(&bytes, buffer, sizeof(int));
  printf("bytes: %d\n", bytes);
  pipeRead(fd, buffer+sizeof(int), pipe_buff, bytes, buffSize);
  int read_counter = 0;
  char * temp = buffer+sizeof(int);
  while (bytes > read_counter) {
    char flag[2] = {0};
    char country[64] = {0};
    char virus[64] = {0};
    char date[64] = {0};
    int stats[4] = {0};
    int counter = 0;
    sscanf(temp+read_counter, "%s", date);
    printf("%s\n", date);
    read_counter += (int)strlen(date)+1;
    sscanf(temp+read_counter, "%s", country);
    printf("%s\n", country);
    read_counter += (int)strlen(country)+1;
    sscanf(temp+read_counter, "%s", flag);
    read_counter += 2;
    while (!strcmp(flag, "T")) {
      sscanf(temp+read_counter, "%s\n", virus);
      printf("%s\n", virus);
      read_counter += (int)strlen(virus)+1;
      sscanf(temp+read_counter, "%d %d %d %d\n", &stats[0], &stats[1], &stats[2], &stats[3]);
      printf("Age range 0-20 years: %d cases\n", stats[0]);
      printf("Age range 21-40 years: %d cases\n", stats[1]);
      printf("Age range 41-60 years: %d cases\n", stats[2]);
      printf("Age range 60+ years: %d cases\n", stats[3]);
      read_counter += 4*sizeof(int)+4;
      sscanf(temp+read_counter, "%s\n", flag);
      read_counter += 2;
    }

  }
  free(pipe_buff);
  return 0;
}
