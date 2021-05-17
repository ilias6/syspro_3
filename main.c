#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions.h"

int main(int argc, char * argv[]) {
  /*The programs needs exactly 8 arguments*/
  if (argc != 9) {
    printf("To run properly:\n./diseaseMonitor -p patientRecordsFile");
    printf("–h1 diseaseHashtableNumOfEntries –h2 countryHashtableNumOfEntries");
    printf("–b bucketSize\n");
    exit(1);
  }

  FILE * f = NULL;
  int numOfEntriesD = 0;
  int numOfEntriesC = 0;
  int bucketSize = 0;

  Param(argv, &f, &numOfEntriesD, &numOfEntriesC, &bucketSize);

  char buffer[100];
  Record r;
  listNode * l = NULL;
  char * eofptr = NULL;
  /*Making the list first*/
  eofptr = fgets(buffer, 100, f);
  for (int i = 0; eofptr != NULL; ++i) {
    if (!setRecord(&r, buffer))
      listInsert(&l, r);
    else
      printf("Error\n");
    eofptr = fgets(buffer, 100, f);
  }

  hashTable * tC = makeTable(numOfEntriesC, bucketSize);
  hashTable * tD = makeTable(numOfEntriesD, bucketSize);
  listNode * temp = l;
  /*Scan the list and create the hash tables too*/
  while (temp != NULL) {
    hashTableInsert(tD, &temp->item, &temp->item.diseaseID);
    hashTableInsert(tC, &temp->item, &temp->item.country);
    temp = temp->next;
  }

  instructions(buffer, l, tD, tC);
  fclose(f);

  return 0;
}
