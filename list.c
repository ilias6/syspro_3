#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "list.h"
#include "functions.h"

Item * findRecord(listNode * head, char * recID) {
  while (head->next != NULL) {
    if (!strcmp(recID, head->item.recordID))
      return &(head->item);
    head = head->next;
  }
  if (head == NULL)
    return NULL;
}

/*Returns null if the recordID already exist*/
Item * listInsert(listNode ** head, Item r) {
  /*First-node insert*/
  if (*head == NULL) {
    *head = malloc(sizeof(listNode));
    if (*head == NULL) {
      printf("Memory Allocation Failure\n(List Insertion 1\n)");
      exit(1);
    }
    (*head)->item = r;
    (*head)->next = NULL;
    return &(*head)->item;
  }
  /*Find the last*/
  /*Check if it already exists!*/
  while ((*head)->next != NULL) {
    if (!strcmp(r.recordID, (*head)->item.recordID)) {
      // printf("Error1\n");
      return NULL;
    }
    head = &((*head)->next);
  }
  if (!strcmp(r.recordID, (*head)->item.recordID)) {
    // printf("Error2\n");
    return NULL;
  }

  listNode * newNode = malloc(sizeof(listNode));
  if (newNode == NULL) {
    printf("Memory Allocation Failure\n(List Insertion 2\n)");
    exit(1);
  }
  newNode->item = r;
  newNode->next = NULL;
  (*head)->next = newNode;
  return &(newNode->item);
}

int makeRecord(listNode ** l, Item * r, char * buff, char * country, char * date) {
  int rIDSize, fNameSize, lNameSize, dIDSize, cSize;
  char rID[20], type[6], fName[20], lName[20], dID[20], a[4];

  if (lineParse(buff) != 6)
    return -1;
  sscanf(buff, "%s %s %s %s %s %s",
    rID, type, fName, lName, dID, a);
  Date d = {0};
  stringToDate(date, &d);
  if (!strcmp(type, "EXIT"))
     return recordPatientExit(*l, rID, d);


  if (countDash(dID) > 1)
    return -1;

  rIDSize = (int)strlen(rID)+1;
  fNameSize = (int)strlen(fName)+1;
  lNameSize = (int)strlen(lName)+1;
  dIDSize = (int)strlen(dID)+1;
  cSize = (int)strlen(country)+1;
  allocateRecord(r, rIDSize, fNameSize, lNameSize, dIDSize, cSize);
  int age = atoi(a);

  memcpy(r->recordID, rID, rIDSize);
  memcpy(r->patientFirstName, fName, fNameSize);
  memcpy(r->patientLastName, lName, lNameSize);
  memcpy(r->diseaseID, dID, dIDSize);
  memcpy(r->country, country, cSize);
  memcpy(&(r->entryDate), &d, sizeof(Date));
  r->age = age;

  return 0;
}

/*Returns (-1) if there is no recordID same as recID
          (-2) if the exit date already exists
          (1) if everything went fine
*/
int listInsertExit(listNode * l, char * recID, Date d) {
  listNode * t = l;
  while (t != NULL && strcmp(recID, t->item.recordID))
    t = t->next;
  if (t == NULL)
    return -1;
  if (dateCompare(t->item.entryDate, d) > 0) {
    printDate(t->item.entryDate);
    return -2;
  }
  if (!dateCompare(t->item.exitDate, d))
    return -2;
  setDate(&t->item.exitDate, d.day, d.month, d.year);
  return 1;
}

void printList(listNode * l) {
  listNode * t = l;
  while (t != NULL) {
    printRecord(t->item);
    printf("\n");
    t = t->next;
  }
}

void listFree(listNode * l) {
  if (l == NULL)
    return;
  listFree(l->next);
  destroyRecord(l->item);
  free(l);
}
