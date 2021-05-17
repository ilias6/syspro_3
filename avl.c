#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "avl.h"

/*This function is needed because we represent
  the height of a null node equal to zero*/
int getHeight(avlNode * node) {
  if (node == NULL)
    return 0;
  return node->H;
}

int max(int n1, int n2) {
  if (n1 > n2)
    return n1;
  return n2;
}

int getBalance(avlNode * node) {
    if (node == NULL)
        return 0;
    return getHeight(node->L) - getHeight(node->R);
}

avlNode * leftRot(avlNode * x) {
  avlNode * y = x->R;
  avlNode * T2 = y->L;

  y->L = x;
  x->R = T2;

  x->H = max(getHeight(x->L), getHeight(x->R))+1;
  y->H = max(getHeight(y->L), getHeight(y->R))+1;

  return y;
}

avlNode * rightRot(avlNode * y) {
  avlNode * x = y->L;
  avlNode * T2 = x->R;

  x->R = y;
  y->L = T2;

  y->H = max(getHeight(y->L), getHeight(y->R))+1;
  x->H = max(getHeight(x->L), getHeight(x->R))+1;

  return x;
}

avlNode * avlNewNode(Record * r) {
  avlNode * node = malloc(sizeof(avlNode));
  node->H = 0;
  node->L = NULL;
  node->R = NULL;
  node->key = r->entryDate;
  node->r = r;
  return node;
}

avlNode * avlInsert(avlNode * root, Record * r) {
  if (root == NULL)
    return avlNewNode(r);

  /*The simple BST insert*/
  if (dateCompare(root->key, r->entryDate) >= 0)
    root->L  = avlInsert(root->L, r);
  else
    root->R = avlInsert(root->R, r);

  root->H = max(getHeight(root->L), getHeight(root->R))+1;

  int balance = getBalance(root);


  /*Left case*/
  if (balance > 1 && dateCompare(root->L->key, r->entryDate) >= 0)
    return rightRot(root);

  /*Left-left case*/
  if (balance > 1 && dateCompare(root->L->key, r->entryDate) < 0) {
    root->L = leftRot(root->L);
    return rightRot(root);
  }

  /*Right case*/
  if (balance < -1 && dateCompare(root->R->key, r->entryDate) < 0)
    return leftRot(root);

  /*Right-right case*/
  if (balance < -1 && dateCompare(root->R->key, r->entryDate) >= 0) {
    root->R = rightRot(root->R);
    return leftRot(root);
  }

  return root;
}

int countAdmissions(avlNode * n, char * virus, Date d1, Date d2) {
  if (n == NULL)
    return 0;
  int count = 0;

  if (!strcmp(virus, n->r->diseaseID))
    if ((dateCompare(n->r->entryDate, d1) >= 0) && (dateCompare(n->r->entryDate, d2) <= 0))
      count = 1;

  count += countAdmissions(n->L, virus, d1, d2);
  count += countAdmissions(n->R, virus, d1, d2);
  return count;
}

int countDischarges(avlNode * n, char * virus, Date d1, Date d2) {
  if (n == NULL)
    return 0;
  int count = 0;
  if (!strcmp(virus, n->r->diseaseID) && (n->r->exitDate.day != 0))
      if ((dateCompare(n->r->exitDate, d1) >= 0) && (dateCompare(n->r->exitDate, d2) <= 0))
        count = 1;

  count += countDischarges(n->L, virus, d1, d2);
  count += countDischarges(n->R, virus, d1, d2);
  return count;
}

void topkAgeSearch(avlNode * n, char * country, Date d1, Date d2, int * stats) {
  if (n == NULL)
    return;
  if (!strcmp(country, n->r->country)) {
    if ((dateCompare(n->r->entryDate, d1) <= 0) && (n->r->exitDate.day == 0)) {
      if (n->r->age <= 20)
        stats[0]++;
      else if (n->r->age <= 40)
        stats[1]++;
      else if (n->r->age <= 60)
        stats[2]++;
      else
        stats[3]++;
    }
    else if ((dateCompare(n->r->entryDate, d1) <= 0) && (dateCompare(n->r->exitDate, d1) >= 0)) {
      if (n->r->age <= 20)
        stats[0]++;
      else if (n->r->age <= 40)
        stats[1]++;
      else if (n->r->age <= 60)
        stats[2]++;
      else
        stats[3]++;
    }
    else if ((dateCompare(n->r->entryDate, d1) >= 0) && (dateCompare(n->r->entryDate, d2) <= 0)) {
      if (n->r->age <= 20)
        stats[0]++;
      else if (n->r->age <= 40)
        stats[1]++;
      else if (n->r->age <= 60)
        stats[2]++;
      else
        stats[3]++;
    }/*
    else if ((dateCompare(n->r->entryDate, d2) > 0)) {
      topkAgeSearch(n->L, country, virus, d1, d2, stats);
      return;
    }*/
  }
  topkAgeSearch(n->L, country, d1, d2, stats);
  topkAgeSearch(n->R, country, d1, d2, stats);
}

void ageSearch(avlNode * n, Date d, char * country, char * virus, int * stats) {
  if (n == NULL)
    return;
  // printf("%s %s %s %s\n", country, n->r->country, virus, n->r->deseaseID);
  if (!strcmp(country, n->r->country) && !strcmp(virus, n->r->diseaseID)) {
    if ((dateCompare(n->r->entryDate, d) <= 0) && (n->r->exitDate.day == 0)) {
      if (n->r->age <= 20)
        stats[0]++;
      else if (n->r->age <= 40)
        stats[1]++;
      else if (n->r->age <= 60)
        stats[2]++;
      else
        stats[3]++;
    }
    else if ((dateCompare(n->r->entryDate, d) <= 0) && (dateCompare(n->r->exitDate, d) >= 0)) {
      if (n->r->age <= 20)
        stats[0]++;
      else if (n->r->age <= 40)
        stats[1]++;
      else if (n->r->age <= 60)
        stats[2]++;
      else
        stats[3]++;
    }
  }
  ageSearch(n->L, d, country, virus, stats);
  ageSearch(n->R, d, country, virus, stats);

}

/*If you DONT uncomment the if cases, a patient is counted only when
  entryDate is between [date1, date2]*/
/*This function is made to satisfy a great part of the program*/
int avlSearch(avlNode * n, Date d1, Date d2, char * country, char * virus) {
  if (n == NULL)
    return 0;
  int count = 0;
  /*Case for globalDiseaseStats*/
  if ((d1.day == 0) && (country[0] == 0) && (virus[0] == 0)) {
     count += avlSearch(n->L, d1, d2, country, virus);
     count += avlSearch(n->R, d1, d2, country, virus);
     return count+1;
  }
  /*Case for numberOfCurrentPatients*/
  else if ((d1.day == -1) && (country[0] == 0) && (virus[0] == 0)) {
    if (n->r->exitDate.day == 0)
      count = 1;
    count += avlSearch(n->L, d1, d2, country, virus);
    count += avlSearch(n->R, d1, d2, country, virus);
    return count;
  }
  /*Second case for globalDiseaseStats*/
  /*(If dates are given)*/
  else if ((country[0] == 0) && (virus[0] == 0)) {
    /*If the entryDate is greater than d2 cut the right branch*/
    if (dateCompare(n->r->entryDate, d2) > 0) {
      count += avlSearch(n->L, d1, d2, country, virus);
      return count;
    }
    /*If entryDate is between [d1,d2]*/
    if ((dateCompare(n->r->entryDate, d1) >= 0) && (dateCompare(d2, n->r->entryDate) >= 0))
      count = 1;
    /*If exitDate is between [d1,d2]*/
    /*
    else if ((dateCompare(n->r->exitDate, d1) >= 0) && (dateCompare(d2, n->r->exitDate)>= 0))
      count = 1;
    /*If the patient has the virus until now*//*
    else if ((dateCompare(d2, n->r->entryDate) >=0) && (n->r->exitDate.day == 0))
      count = 1;
    /*If the patients time period includes the [date1 date2]*//*
    else if ((dateCompare(d2, n->r->entryDate) >=0) && (dateCompare(n->r->exitDate, d2)>= 0))
      count = 1;*/

    count += avlSearch(n->L, d1, d2, country, virus);
    count += avlSearch(n->R, d1, d2, country, virus);
    return count;
  }
  /*Case for topKDiseases*/
  else if ((d1.day == 0) && (virus[0] == 0)) {
    if (!strcmp(country, n->r->country))
      count = 1;
    count += avlSearch(n->L, d1, d2, country, virus);
    count += avlSearch(n->R, d1, d2, country, virus);
    return count;
  }
  /*Second case for topKDiseases*/
  /*(If dates are given)*/
  else if (virus[0] == 0) {
    if (dateCompare(n->r->entryDate, d2) > 0) {
      count += avlSearch(n->L, d1, d2, country, virus);
      return count;
    }
    /*The only difference from above is the condition below*/
    /*We want the patients from a specific country*/
    if (!strcmp(n->r->country, country)) {
      if (dateCompare(n->r->entryDate, d1) >= 0 && dateCompare(d2, n->r->entryDate) >= 0)
        count = 1;
      /*
      else if (dateCompare(n->r->exitDate, d1) >= 0 && dateCompare(d2, n->r->exitDate)>= 0)
        count = 1;
      else if (dateCompare(d2, n->r->entryDate) >=0 && n->r->exitDate.day == 0)
        count = 1;
      else if ((dateCompare(d2, n->r->entryDate) >=0) && (dateCompare(n->r->exitDate, d2)>= 0))
        count = 1;*/
    }

    count += avlSearch(n->L, d1, d2, country, virus);
    count += avlSearch(n->R, d1, d2, country, virus);
    return count;
  }
  /*Case for topKCountries*/
  else if ((d1.day == 0) && (country[0] == 0)) {
    if (!strcmp(virus, n->r->diseaseID))
      count = 1;
    count += avlSearch(n->L, d1, d2, country, virus);
    count += avlSearch(n->R, d1, d2, country, virus);
    return count;
  }
  /*Second case for topKCountries*/
  else {
    if (dateCompare(n->r->entryDate, d2) > 0) {
      count += avlSearch(n->L, d1, d2, country, virus);
      return count;
    }
    if (!strcmp(n->r->diseaseID, virus)) {
      if (dateCompare(n->r->entryDate, d1) >= 0 && dateCompare(d2, n->r->entryDate) >= 0)
        count = 1;
      /*
      else if (dateCompare(n->r->exitDate, d1) >= 0 && dateCompare(d2, n->r->exitDate)>= 0)
        count = 1;
      else if (dateCompare(d2, n->r->entryDate) >=0 && n->r->exitDate.day == 0)
        count = 1;
      else if ((dateCompare(d2, n->r->entryDate) >=0) && (dateCompare(n->r->exitDate, d2)>= 0))
        count = 1;*/
    }
    count += avlSearch(n->L, d1, d2, country, virus);
    count += avlSearch(n->R, d1, d2, country, virus);
    return count;
  }

}

void avlFree(avlNode * root) {
  if (root == NULL)
    return;
  avlFree(root->L);
  avlFree(root->R);
  free(root);
}
