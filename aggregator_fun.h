#ifndef AGGREGATOR_FUN
#define AGGREGATOR_FUN

#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "hash.h"
#include "record.h"

int ext(int, int *, int *, int *);
void masterWaiting(int, int *, int, int *, char *, char *);
int find_worker_index(int *, int, int);
int end_of_work(int, int, int);
void regenerate_worker(int, int *, int, int *, char *, char *);
void instructions(char *, int, int **, int, int *);
void get_answer(int, int, char *, int *, int *);
void globalDiseaseStats(hashTable *, Date, Date);
void insertPatientRecord(listNode *, hashTable *, hashTable *, Record *);
void diseaseFreq(hashTable *, Date, Date, char *, char *);
void numCurrentPatient(hashTable *, char *);
void topKDiseases(hashTable *, int, Date, Date, char *);
void topKCountries(hashTable *, int, Date, Date, char*);
int ageStats(int *, int);

#endif
