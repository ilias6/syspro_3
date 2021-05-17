#ifndef REC
#define REC

#include <stdlib.h>

typedef struct Date {
  int day;
  int month;
  int year;
} Date;

typedef struct Record {
  char * recordID;
  char * patientFirstName;
  char * patientLastName;
  char * diseaseID;
  char * country;
  int age;
  Date entryDate;
  Date exitDate;
} Record;

void allocateRecord(Record *, int, int, int, int, int);
int setRecord(Record *, char *);
void setDate(Date *, int, int, int);
void destroyRecord(Record);
void printRecord(Record);
void printDate(Date);
void printDatetoAns(Date, char *, int *);
void stringToDate(char *, Date *);
int dateCompare(Date, Date);
#endif
