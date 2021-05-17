#ifndef FUN
#define FUN

#include <stdlib.h>

#include "record.h"
#include "hash.h"
#include "list.h"
#include "heap.h"

int dir_counter(char *);
int file_counter(char *);
int safeRead(int, char *, int);
int safeWrite(int, void *, int);
int pipeRead(int, char *, char *, int, int);
int flagsParse(char *, int);
int lineSize(char *);
int recordPatientExit(listNode *, char *, Date);
void GeneralInstructions(void);
int lineParse(char *);
int wordParse(char *, int);
int countDash(char *);

#endif
