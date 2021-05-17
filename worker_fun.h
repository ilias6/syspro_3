#ifndef WORKER_FUN
#define WORKER_FUN

#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "hash.h"

typedef struct worker_space {
  int total;
  int fail;
  int success;
  int n_vir;
  int n_countries;
  int bufferSize;
  listNode * list;
  char * pipe_buff;
  hashTable * tC;
  hashTable * tD;
  char ** viruses;
  char ** countries;
  int ** stats;
  int fd;
  char ** date_files;
  int * file_num;
  char * serverIP;
  int serverPort;
  int ans_sock;
} worker_space;

void update_records(worker_space *);
int write_stats(char *, worker_space *);
void instruction_parse(char *, worker_space *, int, int);
void listCountries(worker_space *);
void searchPatient(worker_space *, char *);
void diseaseFrequency(worker_space *, char *, Date, Date, char *);
void topkAgeRanges(worker_space *, int, char *, char *, Date, Date);
void numAdmissions(worker_space *, char *, Date, Date, char *);
void numDischarges(worker_space *, char *, Date, Date, char *);
char ** get_countries(char *, int, int);
char ** get_date_files(char *, int *);
void sort_date_files(char **, int, int);
void set_up(char *, listNode **, char *, char *);
int end_of_work(char **, int, int, int, int);
void worker_exit(worker_space *);

#endif
