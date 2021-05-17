#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "functions.h"
#include "aggregator_fun.h"

char flag1;
char flag2;
int counter;

int ext(int n, int * fds, int * pids, int * stats) {
  // char readPipe[64] = {0};
  char writePipe[64] = {0};
  for (int i = 0; i < n; ++i) {
    // strcpy(readPipe, "./pipes/rPipe_");
    strcpy(writePipe, "./pipes/wPipe_");
    // sprintf(readPipe+strlen(readPipe), "%d", i);
    sprintf(writePipe+strlen(writePipe), "%d", i);
    // unlink(readPipe);
    unlink(writePipe);
    // close(fds[i][0]);
    close(fds[i]);
  }
  free(fds);
  for (int i = 0; i < n; ++i) {
    kill(pids[i], SIGKILL);
    while (!waitpid(-1, NULL, WNOHANG));
  }
  end_of_work(stats[0], stats[1], stats[2]);

  free(stats);
  free(pids);
  exit(1);
}

int end_of_work(int total, int succ, int fail) {
  char path[64] = {0};
  strcpy(path, "./log_files/log_file.");
  sprintf(path+21, "%d", getpid());

  FILE * f = fopen(path, "w");
  if (f == NULL) {
    perror("Error creating log_file");
    return -1;
  }

  strcpy(path, "./input/\0");
  DIR * dirp = {0};
  struct dirent * dir = {0};
  dirp = opendir(path);

  int n_dirs = dir_counter(path);

  for (int i = 0; i < n_dirs; ++i) {
    dir = readdir(dirp);
    if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
      continue;
    fprintf(f, "%s\n", dir->d_name);
  }
  fprintf(f,"TOTAL %d\nSUCCESS %d\nFAIL %d\n", total, succ, fail);

  closedir(dirp);
  fclose(f);
  return 0;
}

void regenerate_worker(int bufferSize, int * pids, int numOfWorkers, int * fds, char * sip, char * sp) {
  int pid = 0;
  while (!(pid = waitpid(-1, NULL, WNOHANG)));
  int id = find_worker_index(pids, numOfWorkers, pid);
  char workerIndex[5] = {0};
  sprintf(workerIndex, "%d", id);
  char buffsize[5] = {0};
  sprintf(buffsize, "%d", bufferSize);
  if (!(pids[id] = fork()))
    if (execlp("./worker", "./worker", workerIndex, buffsize, (char *)NULL) < 0) {
      perror("EXEC ERROR");
      int * stats = calloc(3, sizeof(int));
      ext(numOfWorkers, fds, pids, stats);
    }

  close(fds[id]);

  // char readPipe[64] = {0};
  char writePipe[64] = {0};
  // strcpy(readPipe, "./pipes/rPipe_");
  strcpy(writePipe, "./pipes/wPipe_");
  // sprintf(readPipe+strlen(readPipe), "%d", id);
  sprintf(writePipe+strlen(writePipe), "%d", id);

  /*if ((fds[id][0] = open(readPipe, O_RDONLY | O_NONBLOCK)) < 0) {
    perror("Error opening rpipe1");
    int * stats = calloc(3, sizeof(int));
    ext(numOfWorkers, fds, pids, stats);
  }*/

  while ((fds[id] = open(writePipe, O_WRONLY | O_NONBLOCK | O_TRUNC)) < 0);

  char path[64] = {0};
  strcpy(path, "./");
  strcpy(path+strlen(path), "./input/");
  /*. and .. is there too!*/
  int countriesNum = dir_counter(path) -2;
  int countriesPerWorker = countriesNum/numOfWorkers;
  int more = countriesNum%numOfWorkers;
  int low = 0;
  int high = 0;

  if (id+1 > more) {
    low = id*countriesPerWorker+more;
    high = low + countriesPerWorker-1;
  }
  else if (id+1 == more) {
    low = id*countriesPerWorker+more;
    high = low + countriesPerWorker;
  }
  else {
    low = id*countriesPerWorker+id;
    high = low + countriesPerWorker;
  }
  char buffer[1024];
  int len = (int)strlen(path);
  memcpy(buffer, &len, sizeof(int));
  memcpy(buffer+sizeof(int), path, len);
  memcpy(buffer+sizeof(int)+len, &low, sizeof(int));
  memcpy(buffer+2*sizeof(int)+len, &high, sizeof(int));

  write(fds[id], buffer, 3*sizeof(int)+len);

  int len1 = strlen(sip)+1;
  int len2 = strlen(sp)+1;
  write(fds[id], &len1, sizeof(int));
  write(fds[id], sip, len1);
  write(fds[id], &len2, sizeof(int));
  write(fds[id], sp, len2);
  // ageStats(fds[id], bufferSize);
}

int find_worker_index(int * pids, int n, int pid) {
  for (int i = 0; i < n; ++i)
    if (pid == pids[i])
      return i;
  return -1;
}

void masterWaiting(int bufferSize, int  * pids, int numOfWorkers, int * fds, char * sip, char * sp) {
  while (1) {
    /*One worker is dead*/
    if (counter) {
      /*Maybe all the workers are dead*/
      sleep(2);
      if (counter == 1) {
        regenerate_worker(bufferSize, pids, numOfWorkers, fds, sip, sp);
        counter = 0;
      }
    }
    /*SIGINT caught*/
    if (flag1) {
      int * stats = calloc(3, sizeof(int));
      ext(numOfWorkers, fds, pids, stats);
    }
  }
}

/*The options while the program is running*/
void instructions(char * buffer, int numOfWorkers, int ** fds, int bufferSize, int * pids) {
  /*Used for the log_file*/
  int * stats = calloc(3, sizeof(int));
  char * pipe_buff = malloc(bufferSize);
  struct timeval tv = {0};
  tv.tv_usec = 100;
  fd_set rfds;

  for (int i = 0; i < 4096; ++i)
    buffer[i] = 0;

  char * flags = calloc(numOfWorkers, sizeof(char));
  size_t size = 4096;
  int retval = 0;
  char * ptr = buffer;

  /*If every flag is 1, all workers have send summary statistics*/
  /*while (flagsParse(flags, numOfWorkers)) {
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    /*While waiting for summary statistics, take instructions too!*/
    /*retval = select(1, &rfds, NULL, NULL, &tv);
    if (retval > 0) {
      int n = getline(&ptr, &size, stdin);
      size -= n;
      ptr += n;
    }
    else if (retval == -1) {
      perror("select");
      // ext(numOfWorkers, fds, pids , stats);
    }
    /*for (int i = 0; i < numOfWorkers; ++i) {
      /*If worker i has already sent*/
      /*if (flags[i])
        continue;
      FD_ZERO(&rfds);
      FD_SET(fds[i][0], &rfds);
      retval = select(fds[i]+1, &rfds, NULL, NULL, &tv);
      if (retval > 0) {
        ageStats(fds[i], bufferSize);
        flags[i] = 1;
      }
      else if (retval == -1) {
        perror("select");
        // ext(numOfWorkers, fds, pids, stats);
      }
    }*/
  // }
  /*The next use of the flags[] is for diseaseFrequency*/
  for (int i = 0; i < numOfWorkers; ++i)
    flags[i] = 0;

  ptr = buffer;
  char * line = NULL;
  int bytes = 0;
  char miniBuffer[128] = {0};
  int count = 0;
  while (1) {
    /*One worker is dead*/
    if (counter) {
      /*Maybe all the workers are dead*/
      sleep(2);
      if (counter == 1) {
        // regenerate_worker(bufferSize, pids, numOfWorkers, fds);
        counter = 0;
      }
    }
    /*SIGINT caught*/
    if (flag1) {
      free(pipe_buff);
      free(flags);
      // ext(numOfWorkers, fds, pids, stats);
    }
    /*FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    retval = select(1, &rfds, NULL, NULL, &tv);
    if (flag1) {
      free(pipe_buff);
      free(flags);
      ext(numOfWorkers, fds, pids, stats);
    }
    if ((line = strtok(ptr, "\n")) != NULL) {
      bytes = lineSize(line);
      ptr += bytes;
      memcpy(miniBuffer, line, lineSize(line));
    }
    else {
      if (flag1) {
        free(pipe_buff);
        free(flags);
        ext(numOfWorkers, fds, pids, stats);
      }
      if (counter) {
        sleep(2);
        if (counter == 1) {
          regenerate_worker(bufferSize, pids, numOfWorkers, fds);
          counter = 0;
        }
      }
      fgets(miniBuffer, 128, stdin);
      if (counter) {
        sleep(2);
        if (counter == 1) {
          regenerate_worker(bufferSize, pids, numOfWorkers, fds);
          counter = 0;
        }
      }
      if (flag1) {
        free(pipe_buff);
        free(flags);
        ext(numOfWorkers, fds, pids, stats);
      }
      /*Size of instruction*/
      // bytes = lineSize(miniBuffer);
    // }

    /*for (int i = 0; i < numOfWorkers; ++i) {
      FD_ZERO(&rfds);
      FD_SET(fds[i][0], &rfds);
      retval = select(fds[i][0]+1, &rfds, NULL, NULL, &tv);
      if (counter) {
        sleep(2);
        if (counter == 1) {
          regenerate_worker(bufferSize, pids, numOfWorkers, fds);
          counter = 0;
        }
      }
      if (flag1) {
        free(pipe_buff);
        free(flags);
        ext(numOfWorkers, fds, pids, stats);
      }
      /*Someone answered*/
      /*if (retval > 0) {
        get_answer(fds[i][0], bufferSize, &flags[i], &count, stats);
      }
      else if (retval == -1) {
        if (counter) {
          sleep(2);
          if (counter == 1) {
            regenerate_worker(bufferSize, pids, numOfWorkers, fds);
            counter = 0;
          }
        }
        if (flag1) {
          free(pipe_buff);
          free(flags);
          ext(numOfWorkers, fds, pids, stats);
        }
        perror("select");
      }
    }
    /*All diseaseFrequency number are collected*/
    /*if (!flagsParse(flags, numOfWorkers)) {
      for (int i = 0; i < numOfWorkers; ++i)
        flags[i] = 0;
      printf("%d\n", count);
    }

    char instr[50] = {0};
    sscanf(miniBuffer, "%s", instr);
    Date d1 = {0};
    Date d2 = {0};
    char str0[20] = {0};
    char str1[20] = {0};
    char str2[20] = {0};
    char str3[20] = {0};
    char str4[20] = {0};
    if (counter) {
      sleep(2);
      if (counter == 1) {
        regenerate_worker(bufferSize, pids, numOfWorkers, fds);
        counter = 0;
      }
    }
    if (flag1) {
      free(pipe_buff);
      free(flags);
      ext(numOfWorkers, fds, pids, stats);
    }
    if (!strcmp(instr, "/exit")) {
      printf("exiting\n");
      free(pipe_buff);
      free(flags);
      ext(numOfWorkers, fds, pids, stats);
    }

    bytes += 4;
    if (!strcmp(instr, "/listCountries")) {
        for (int i = 0; i < numOfWorkers; ++i) {
          write(fds[i][1], &bytes, sizeof(int));
          write(fds[i][1], instr, (int)strlen(instr)+1);
          write(fds[i][1], "END", 4);
        }
    }
    else if (!strcmp(instr, "/diseaseFrequency")) {
      count = 0;
      if (lineParse(miniBuffer) == 4) {
        sscanf(miniBuffer+strlen(instr)+1, "%s %s %s", str0, str1, str2);
        for (int i = 0; i < numOfWorkers; ++i) {
          write(fds[i][1], &bytes, sizeof(int));
          write(fds[i][1], instr, (int)strlen(instr)+1);
          write(fds[i][1], str0, (int)strlen(str0)+1);
          write(fds[i][1], str1, (int)strlen(str1)+1);
          write(fds[i][1], str2, (int)strlen(str2)+1);
          write(fds[i][1], "END", 4);
        }
      }
      else if (lineParse(miniBuffer) == 5) {
        sscanf(miniBuffer+strlen(instr)+1, "%s %s %s %s", str0, str1, str2, str3);
        for (int i = 0; i < numOfWorkers; ++i) {
          write(fds[i][1], &bytes, sizeof(int));
          write(fds[i][1], instr, (int)strlen(instr)+1);
          write(fds[i][1], str0, (int)strlen(str0)+1);
          write(fds[i][1], str1, (int)strlen(str1)+1);
          write(fds[i][1], str2, (int)strlen(str2)+1);
          write(fds[i][1], str3, (int)strlen(str3)+1);
          write(fds[i][1], "END", 4);
        }
      }

    }
    else if (!strcmp(instr, "/searchPatientRecord")) {
      if (lineParse(miniBuffer) == 2) {
        sscanf(miniBuffer+strlen(instr)+1, "%s", str0);
        for (int i = 0; i < numOfWorkers; ++i) {
          write(fds[i][1], &bytes, sizeof(int));
          write(fds[i][1], instr, (int)strlen(instr)+1);
          write(fds[i][1], str0, (int)strlen(str0)+1);
          write(fds[i][1], "END", 4);
        }
      }
    }
    else if (!strcmp(instr, "/topk-AgeRanges")) {
      if (lineParse(miniBuffer) == 6) {
        sscanf(miniBuffer+strlen(instr)+1, "%s %s %s %s %s", str0, str1, str2, str3, str4);
        for (int i = 0; i < numOfWorkers; ++i) {
          write(fds[i][1], &bytes, sizeof(int));
          write(fds[i][1], instr, (int)strlen(instr)+1);
          write(fds[i][1], str0, (int)strlen(str0)+1);
          write(fds[i][1], str1, (int)strlen(str1)+1);
          write(fds[i][1], str2, (int)strlen(str2)+1);
          write(fds[i][1], str3, (int)strlen(str3)+1);
          write(fds[i][1], str4, (int)strlen(str4)+1);
          write(fds[i][1], "END", 4);
        }
      }
    }
    else if (!strcmp(instr, "/numPatientAdmissions")) {
      if (lineParse(miniBuffer) == 4) {
        sscanf(miniBuffer+strlen(instr)+1, "%s %s %s", str0, str1, str2);
        for (int i = 0; i < numOfWorkers; ++i) {
          write(fds[i][1], &bytes, sizeof(int));
          write(fds[i][1], instr, (int)strlen(instr)+1);
          write(fds[i][1], str0, (int)strlen(str0)+1);
          write(fds[i][1], str1, (int)strlen(str1)+1);
          write(fds[i][1], str2, (int)strlen(str2)+1);
          write(fds[i][1], "END", 4);
        }
      }
      else if (lineParse(miniBuffer) == 5) {
        sscanf(miniBuffer+strlen(instr)+1, "%s %s %s %s", str0, str1, str2, str3);
        for (int i = 0; i < numOfWorkers; ++i) {
          write(fds[i][1], &bytes, sizeof(int));
          write(fds[i][1], instr, (int)strlen(instr)+1);
          write(fds[i][1], str0, (int)strlen(str0)+1);
          write(fds[i][1], str1, (int)strlen(str1)+1);
          write(fds[i][1], str2, (int)strlen(str2)+1);
          write(fds[i][1], str3, (int)strlen(str3)+1);
          write(fds[i][1], "END", 4);
        }
      }
    }
    else if (!strcmp(instr, "/numPatientDischarges")) {
      if (lineParse(miniBuffer) == 4) {
        sscanf(miniBuffer+strlen(instr)+1, "%s %s %s", str0, str1, str2);
        for (int i = 0; i < numOfWorkers; ++i) {
          write(fds[i][1], &bytes, sizeof(int));
          write(fds[i][1], instr, (int)strlen(instr)+1);
          write(fds[i][1], str0, (int)strlen(str0)+1);
          write(fds[i][1], str1, (int)strlen(str1)+1);
          write(fds[i][1], str2, (int)strlen(str2)+1);
          write(fds[i][1], "END", 4);
        }
      }
      else if (lineParse(miniBuffer) == 5) {
        sscanf(miniBuffer+strlen(instr)+1, "%s %s %s %s", str0, str1, str2, str3);
        for (int i = 0; i < numOfWorkers; ++i) {
          write(fds[i][1], &bytes, sizeof(int));
          write(fds[i][1], instr, (int)strlen(instr)+1);
          write(fds[i][1], str0, (int)strlen(str0)+1);
          write(fds[i][1], str1, (int)strlen(str1)+1);
          write(fds[i][1], str2, (int)strlen(str2)+1);
          write(fds[i][1], str3, (int)strlen(str3)+1);
          write(fds[i][1], "END", 4);
        }
      }
    }
    else {
      GeneralInstructions();
    }*/
  }

}

void get_answer(int fd, int buffSize, char * flag, int * count, int * stats) {
  char buffer[4096] = {0};
  char * pipe_buff = malloc(buffSize);
  stats[0]++;
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
      ++stats[1];
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
        stats[2]++;
        break;
      }
      stats[1]++;
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
      stats[1]++;
      *flag = 1;
      int n = 0;
      memcpy(&n, ptr, sizeof(int));
      (*count) += n;
      break;
    case 3:
      if (bytes == 6) {
        stats[2]++;
        break;
      }
      stats[1]++;
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
        stats[2]++;
        break;
      }
      stats[2]++;
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
        stats[2]++;
        break;
      }
      stats[1]++;
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
      stats[1]++;
      while (bytes > 0) {
        char flag[2] = {0};
        char country[64] = {0};
        char virus[64] = {0};
        char date[64] = {0};
        int stats[4] = {0};
        int counter = 0;
        sscanf(ptr, "%s", date);
        printf("%s\n", date);
        bytes -= (int)strlen(date)+1;
        ptr += (int)strlen(date)+1;
        sscanf(ptr, "%s", country);
        printf("%s\n", country);
        bytes -= (int)strlen(country)+1;
        ptr += (int)strlen(country)+1;
        sscanf(ptr, "%s", flag);
        bytes -= 2;
        ptr += 2;
        while (!strcmp(flag, "T")) {
          sscanf(ptr, "%s\n", virus);
          printf("%s\n", virus);
          bytes -= (int)strlen(virus)+1;
          ptr += (int)strlen(virus)+1;
          sscanf(ptr, "%d %d %d %d\n", &stats[0], &stats[1], &stats[2], &stats[3]);
          printf("Age range 0-20 years: %d cases\n", stats[0]);
          printf("Age range 21-40 years: %d cases\n", stats[1]);
          printf("Age range 41-60 years: %d cases\n", stats[2]);
          printf("Age range 60+ years: %d cases\n", stats[3]);
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

int ageStats(int * fds, int buffSize) {
  char buffer[8096] = {0};
  char * pipe_buff = malloc(buffSize);

  pipeRead(fds[0], buffer, pipe_buff, sizeof(int), buffSize);
  int bytes = 0;
  memcpy(&bytes, buffer, sizeof(int));
  // printf("bytes: %d\n", bytes);
  pipeRead(fds[0], buffer+sizeof(int), pipe_buff, bytes, buffSize);
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
