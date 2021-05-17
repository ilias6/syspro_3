#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>

#include "functions.h"

/*In case the input is invalid*/
void GeneralInstructions(void) {
  printf("Your options are:\n");
  printf("1./listCountries\n");
  printf("2./diseaseFrequency virusname date1 date2 [country]\n");
  printf("3./topk-AgeRanges k country disease date1 date2\n");
  printf("4./searchPatientRecord recordID\n");
  printf("5./numPatientAdmissions disease date1 date2 [country] \n");
  printf("6./numPatientDischarges disease date1 date2 [country] \n");
  printf("7./exit\n");
}

int lineSize(char * line) {
  int size = 0;
  while (*line != '\n') {
    if (line == NULL)
      return -1;
    ++line;
    ++size;
  }
  return size+1;
}

int recordPatientExit(listNode * l, char * recID, Date d) {
   return listInsertExit(l, recID, d);
  /*else
    printf("Record updated\n");*/
}

int safeWrite(int fd, void * buffer, int len) {
  int count = 0;
  int bytes = 0;
  while (count < len) {
    if ((bytes = write(fd, buffer, len)) < 0) {
      perror("write");
      return -1;
    }
    count += bytes;
  }
  return count;
}

int pipeRead(int fd, char * buffer, char * pipe_buff, int size, int buff_size) {
  int cur_read = 0;
  char * buff_ptr = buffer;
  while (cur_read < size) {
    if ((size-cur_read) > buff_size) {
      cur_read += safeRead(fd, pipe_buff, buff_size);
      memcpy(buff_ptr, pipe_buff, buff_size);
    }
    else {
      int x = size-cur_read;
      cur_read += safeRead(fd, pipe_buff, size-cur_read);
      memcpy(buff_ptr, pipe_buff, x);
    }
    buff_ptr = buffer+cur_read;
  }
  return cur_read;
}

int safeRead(int fd, char * buffer, int rSize) {
  int size = 0;
  int flag = 0;
  while (size < rSize) {
    flag = read(fd, buffer+size, rSize-size);
    if (flag < 0)
      continue;
    size += flag;
  }
  return size;
}


int flagsParse(char * flags, int n) {
  for (int i = 0; i < n; ++i)
    if (!flags[i])
      return 1;
  return 0;
}

/*Used above*/
/*Counts the words of a read line*/
int lineParse(char * line) {
  int len = strlen(line);
  int count = 0;
  for (int i = 0; i < len; ++i) {
    if (i > 0) {
      if ((line[i-1] == ' ') && (line[i] != ' ' && line[i] != '\n'))
        ++count;
    }
    else {
      if (line[0] != ' ')
        ++count;
    }
  }
  return count;
}

int countDash(char * word) {
  int len = strlen(word);
  int count = 0;
  for (int i = 0; i < len; ++i)
    if (word[i] == '-')
      ++count;
  return count;
}

int ifLetter(char c) {
  if (c >= 'A' && c <= 'z')
    return 1;
  return 0;
}

int ifNum(char n) {
  if (n >= '0' && n <= '9')
    return 1;
  return 0;
}
/*If flag == 0 --> check for letters*/
/*If flag == 1 --> check for letters and one number*/
/*If flag == 2 --> same as flag == 2, but also count dashes*/
/*Returns 0 when it succeeds*/
/*Otherwise -1*/
int wordParse(char * word, int flag) {
  int len = strlen(word);
  if (flag == 0) {
    for (int i = 0; i < len; ++i)
      if (!ifLetter(word[i]))
        return -1;
  }
  else if (flag == 1) {
    int numCount = 0;
    for (int i = 0; i < len; ++i) {
      if (ifNum(word[i])) {
        if (numCount > 0)
          return -1;
        else
          ++numCount;
      }
      else if (!ifLetter(word[i]) && !ifNum(word[i]))
        return -1;
    }
  }
  else if (flag == 2) {
    if (countDash(word) > 1)
      return -1;
    int numCount = 0;
    for (int i = 0; i < len; ++i) {
      if (ifNum(word[i])) {
        if (numCount > 0)
          return -1;
        else
          ++numCount;
      }
      else if (!ifLetter(word[i]) && !ifNum(word[i]))
        return -1;
    }
  }
  return 0;
}

int file_counter(char * path) {
  int file_count = 0;
  DIR * dirp = {0};
  struct dirent * entry = {0};

  dirp = opendir(path);
  if (dirp < 0) {
    perror("opendir");
    exit(1);
  }

  while ((entry = readdir(dirp)) != NULL)
    if (entry->d_type == DT_REG)
      file_count++;

  closedir(dirp);
  return file_count;
}

int dir_counter(char * path) {
  int dir_count = 0;
  DIR * dirp = {0};
  struct dirent * entry = {0};

  dirp = opendir(path);
  if (dirp < 0) {
    perror("opendir");
    exit(1);
  }

  while ((entry = readdir(dirp)) != NULL)
    if (entry->d_type == DT_DIR)
      dir_count++;

  closedir(dirp);
  return dir_count;
}

void Exit(listNode * l, hashTable * tD, hashTable * tC) {
  listFree(l);
  freeHashTable(tC);
  freeHashTable(tD);
}
