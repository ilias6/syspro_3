#include <stdio.h>
#include <stdlib.h>
#include "record.h"
#include "functions.h"
#include "string.h"

void printDate(Date d) {
  if (d.day == 0) {
    printf("-\n");
    return;
  }
  printf("%d-%d-%d\n", d.day, d.month, d.year);
}

void printDatetoAns(Date d, char * ans, int * val) {
  if (d.day == 0) {
    sprintf(ans, "-\n");
    *val += 3;
    return;
  }
  sprintf(ans, "%d-%d-%d\n", d.day, d.month, d.year);
  *val += sizeof(int)*3+4;
}

/*Returns:  (1) if d1 is greater
            (0) if they are equal
            (-1) if d2 is greater
*/
int dateCompare(Date d1, Date d2) {
  if (d1.year > d2.year)
    return 1;
  else if (d1.year < d2.year)
    return -1;

  if (d1.month > d2.month)
    return 1;
  else if (d1.month < d2.month)
    return -1;

  if (d1.day > d2.day)
    return 1;
  else if (d1.day < d2.day)
    return -1;


  return 0;
}

void printRecord(Record r) {
  printf("ID: %s\nFirst name: %s\nLast name: %s\ndisease: %s\ncountry: %s\nage: %d\n",
    r.recordID, r.patientFirstName, r.patientLastName, r.diseaseID, r.country, r.age);
  printf("entry date: ");
  printDate(r.entryDate);
  printf("exit date: ");
  printDate(r.exitDate);
}

void setDate(Date * d, int day, int month, int year) {
  d->day = day;
  d->month = month;
  d->year = year;
}

void allocateRecord(Record * r, int recordIDSize, int firstNameSize, int lastNameSize,
   int diseaseIDSize, int countrySize) {

  r->recordID = malloc(recordIDSize*sizeof(char));
  r->patientFirstName = malloc(firstNameSize*sizeof(char));
  r->patientLastName = malloc(lastNameSize*sizeof(char));
  r->diseaseID = malloc(diseaseIDSize*sizeof(char));
  r->country = malloc(countrySize*sizeof(char));
}

void stringToDate(char * str, Date * d) {
  char t[4] = {0};
  strncpy(t, str, 2);
  d->day = atoi(t);
  strncpy(t, str+3, 2);
  d->month = atoi(t);
  strncpy(t, str+6, 4);
  d->year = atoi(t);
}

int setRecord(Record * r, char * str) {
  int rIDSize, fNameSize, lNameSize, dIDSize, cSize;
  char rID[20], fName[20], lName[20], dID[20], c[20], d1[20], d2[20];

  d2[0] = 0;
  d2[1] = 0;
  if (lineParse(str) > 8 && lineParse(str) < 7)
    return -1;
  sscanf(str, "%s %s %s %s %s %s %s",
      rID, fName, lName, dID, c, d1, d2);
  if (countDash(dID) > 1)
    return -1;
  rIDSize = (int)strlen(rID)+1;
  fNameSize = (int)strlen(fName)+1;
  lNameSize = (int)strlen(lName)+1;
  dIDSize = (int)strlen(dID)+1;
  cSize = (int)strlen(c)+1;
  allocateRecord(r, rIDSize, fNameSize, lNameSize, dIDSize, cSize);

  memcpy(r->recordID, rID, rIDSize);
  memcpy(r->patientFirstName, fName, fNameSize);
  memcpy(r->patientLastName, lName, lNameSize);
  memcpy(r->diseaseID, dID, dIDSize);
  memcpy(r->country, c, cSize);
  Date date = {0};
  stringToDate(d1, &date);
  memcpy(&(r->entryDate), &date, sizeof(Date));
  if (*d2 == '-' || (d2[0] == 0 && d2[1] == 0)) {
    date.day = 0;
    date.month = 0;
    date.year = 0;
  }
  else {
    stringToDate(d2, &date);
  }
  memcpy(&(r->exitDate), &date, sizeof(Date));

  if (dateCompare(r->exitDate, r->entryDate) == -1) {
    destroyRecord(*r);
    return -1;
  }
  return 0;
}

void destroyRecord(Record r) {
  free(r.recordID);
  free(r.patientFirstName);
  free(r.patientLastName);
  free(r.diseaseID);
  free(r.country);
}
