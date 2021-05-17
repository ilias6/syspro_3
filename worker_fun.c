#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include "functions.h"
#include "worker_fun.h"

char flag1;
char flag2;

#define N_ENTRIES_C 10
#define N_ENTRIES_D 10

void instruction_parse(char * buffer, worker_space * ws, int size, int sock) {
    char instr[32] = {0};
    char * ptr = NULL;
    ws->total++;
    sscanf(buffer, "%s", instr);
    ptr = buffer+strlen(instr)+1;
    if (!strcmp(instr, "/listCountries")) {
      listCountries(ws);
    }
    else if (!strcmp(instr, "/topk-AgeRanges"))  {
      char country[32] = {0};
      char disease[32] = {0};
      char str0[11] = {0};
      char str1[11] = {0};
      char str2[4] = {0};
      Date d1 = {0};
      Date d2 = {0};
      sscanf(ptr, "%s", str0);
      int k = atoi(str0);

      ptr += strlen(str0)+1;
      sscanf(ptr, "%s", country);
      ptr += strlen(country)+1;
      sscanf(ptr, "%s", disease);
      ptr += strlen(disease)+1;
      sscanf(ptr, "%s", str1);
      ptr += strlen(str1)+1;
      sscanf(ptr, "%s", str2);

      stringToDate(str1, &d1);
      stringToDate(str2, &d2);
      topkAgeRanges(ws, k, country, disease, d1, d2);
    }
    else if (!strcmp(instr, "/diseaseFrequency")) {
      char country[32] = {0};
      char disease[32] = {0};
      char str0[11] = {0};
      char str1[11] = {0};
      Date d1 = {0};
      Date d2 = {0};
      int size = lineParse(ptr);
      // printf("size:%d %s END\n",size, ptr);
      sscanf(ptr, "%s", disease);
      ptr += strlen(disease)+1;
      sscanf(ptr, "%s", str0);
      ptr += strlen(str0)+1;
      sscanf(ptr, "%s", str1);
      ptr += strlen(str1)+1;
      stringToDate(str0, &d1);
      stringToDate(str1, &d2);
      if (size == 3) {
        diseaseFrequency(ws, disease, d1, d2, "\0\0\0");
      }
      else {
        sscanf(ptr, "%s", country);
        diseaseFrequency(ws, disease, d1, d2, country);
      }

    }
    else if (!strcmp(instr, "/searchPatientRecord")) {
      char id[32] = {0};
      sscanf(ptr, "%s", id);
      searchPatient(ws, id);
    }
    else if (!strcmp(instr, "/numPatientAdmissions")) {
      char country[32] = {0};
      char disease[32] = {0};
      char str0[11] = {0};
      char str1[11] = {0};
      Date d1 = {0};
      Date d2 = {0};
      int size = lineParse(ptr);
      sscanf(ptr, "%s", disease);
      ptr += strlen(disease)+1;
      sscanf(ptr, "%s", str0);
      ptr += strlen(str0)+1;
      sscanf(ptr, "%s", str1);
      ptr += strlen(str1)+1;
      stringToDate(str0, &d1);
      stringToDate(str1, &d2);
      if (size == 3) {
        numAdmissions(ws, disease, d1, d2, NULL);
      }
      else {
        sscanf(ptr, "%s", country);
        numAdmissions(ws, disease, d1, d2, country);
      }
    }
    else if (!strcmp(instr, "/numPatientDischarges")) {
      char country[32] = {0};
      char disease[32] = {0};
      char str0[11] = {0};
      char str1[11] = {0};
      Date d1 = {0};
      Date d2 = {0};
      int size = lineParse(ptr);
      sscanf(ptr, "%s", disease);
      ptr += strlen(disease)+1;
      sscanf(ptr, "%s", str0);
      ptr += strlen(str0)+1;
      sscanf(ptr, "%s", str1);
      ptr += strlen(str1)+1;
      stringToDate(str0, &d1);
      stringToDate(str1, &d2);
      if (size == 3) {
        numDischarges(ws, disease, d1, d2, NULL);
      }
      else {
        sscanf(ptr, "%s", country);
        numDischarges(ws, disease, d1, d2, country);
      }
    }
}

int write_stats(char * buffer, worker_space * ws) {
  char * ptr = buffer+sizeof(int);
  int ** stats = malloc(ws->n_vir*sizeof(int *));
  for (int i = 0; i < ws->n_vir; ++i) {
    stats[i] = malloc(sizeof(int)*4);
    for (int j = 0; j < 4; ++j)
      stats[i][j] = 0;
  }
  int * file_num = malloc(sizeof(int)*ws->n_countries);
  int tag = 6;
  memcpy(ptr, &tag, sizeof(int));
  int bytes = sizeof(int);
  char tr[2] = "T";
  char fa[2] = "F";
  char path[64] = {0};
  strcpy(path, "./input/");
  int len = strlen(path);
  for (int i = 0; i < ws->n_countries; ++i) {
    strcpy(path+len, ws->countries[i]);
    file_num[i] = 0;
    char ** date_files = get_date_files(path, &file_num[i]);
    sort_date_files(date_files, 0, file_num[i]-1);
    for (int j = 0; j < file_num[i]; ++j) {
      sprintf(ptr+bytes, "%s\n", date_files[j]);
      bytes += (int)strlen(date_files[j])+1;
      sprintf(ptr+bytes, "%s\n", ws->countries[i]);
      bytes += (int)strlen(ws->countries[i])+1;
      Date date = {0};
      stringToDate(date_files[j], &date);
      int count = 0;
      for (int k = 0; k < N_ENTRIES_D; ++k)
        ageStatistics(ws->tD->table[k], date, ws->countries[i], stats, &count);
      for (int v = 0; v < ws->n_vir; ++v) {
        if (!(stats[v][0] == 0 && stats[v][1] == 0 && stats[v][2] == 0 && stats[v][3] == 0)){
          sprintf(ptr+bytes, "%s\n", tr);
          bytes += 2;
          sprintf(ptr+bytes, "%s\n", ws->viruses[v]);
          bytes += (int)strlen(ws->viruses[v])+1;
        }
        else {
          continue;
        }
        sprintf(ptr+bytes, "%d %d %d %d\n", stats[v][0], stats[v][1], stats[v][2], stats[v][3]);
        bytes += 4*sizeof(int)+4;
        stats[v][0] = 0;
        stats[v][1] = 0;
        stats[v][2] = 0;
        stats[v][3] = 0;
      }
      sprintf(ptr+bytes, "%s\n", fa);
      bytes += 2;
    }
    for (int j = 0; j < file_num[i]; ++j)
      free(date_files[j]);
    free(date_files);
  }
  free(file_num);
  return bytes;
}

void update_records(worker_space * ws) {
  flag2 = 0;
  char path[64] = {0};
  strcpy(path, "./input/");
  int len = strlen(path);
  int * file_num = malloc(sizeof(int)*ws->n_countries);
  for (int i = 0; i < ws->n_countries; ++i) {
    strcpy(path+len, ws->countries[i]);
    char ** date_files = get_date_files(path, &file_num[i]);
    sort_date_files(date_files, 0, file_num[i]-1);
    strcpy(path+len+strlen(ws->countries[i]), "/");
    for (int j = 0; j < file_num[i]; ++j) {
      strcpy(path+len+strlen(ws->countries[i])+1, date_files[j]);
      char buffer[128] = {0};
      char * eofptr = NULL;

      Record r = {0};
      FILE * f = fopen(path, "r");
      eofptr = fgets(buffer, 128, f);
      int val = 0;
      Record * rptr = NULL;

      while (eofptr != NULL){
        if (!(val = makeRecord(&(ws->list), &r, buffer, ws->countries[i], date_files[j])))
          rptr = listInsert(&(ws->list), r);
        if ((val == 1) | ((!val) && (rptr != NULL))) {
          char buff[4096] = {0};
          int bytes = write_stats(buff, ws);
          memcpy(buff, &bytes, sizeof(int));
          safeWrite(ws->fd, buff, bytes+sizeof(int));
        }
        eofptr = fgets(buffer, 128, f);
      }
      fclose(f);
    }
    for (int j = 0; j < file_num[i]; ++j)
      free(date_files[j]);
    free(date_files);
  }
  free(file_num);

}

void listCountries(worker_space * ws) {
  char buffer[256] = {0};
  ws->fail++;
  char * ptr = buffer;
  int size = sizeof(int);
  int tag = 0;
  memcpy(buffer+size, &tag, sizeof(int));
  size += sizeof(int);
  int pid = getpid();
  memcpy(ptr+size, &pid, sizeof(int));
  size += sizeof(int);
  for (int i = 0; i < ws->n_countries; ++i) {
    int len = (int)strlen(ws->countries[i])+1;
    memcpy(ptr+size, &len, sizeof(int));
    size += sizeof(int);
    memcpy(ptr+size, ws->countries[i], len);
    size += len;
  }
  size -= sizeof(int);
  memcpy(buffer, &size, sizeof(int));
  safeWrite(ws->ans_sock, buffer, size+sizeof(int));
  ws->fail--;
  ws->success++;

}

void diseaseFrequency(worker_space * ws, char * virus, Date d1, Date d2, char * country) {
  int count = diseaseFreqb(ws->tD->table[bIndex(*(ws->tD), virus)], d1, d2, virus, country);
  if (count == 0)
    ws->fail++;
  else
    ws->success++;
  char buffer[32] = {0};
  int size = 2*sizeof(int);
  int tag = 2;
  memcpy(buffer, &size, sizeof(int));
  memcpy(buffer+sizeof(int), &tag, sizeof(int));
  memcpy(buffer+2*sizeof(int), &count, sizeof(int));
  safeWrite(ws->ans_sock, buffer, size+sizeof(int));
}

int countryParse(char ** countries, int n, char * country) {
  for (int i = 0; i < n; ++i)
    if (!strcmp(countries[i], country))
      return 1;
  return 0;
}

void topkAgeRanges(worker_space * ws, int k, char * country, char * disease, Date d1, Date d2) {
  if (!countryParse(ws->countries, ws->n_countries, country)) {
    ws->fail++;
    int bytes = 6+sizeof(int);
    safeWrite(ws->ans_sock, &bytes, sizeof(int));
    int tag = 1;
    safeWrite(ws->ans_sock, &tag, sizeof(int));
    safeWrite(ws->ans_sock, "ERROR", 6);
    return;
  }
  avlNode * tree = NULL;
  tree = virusTree(ws->tD->table[bIndex(*(ws->tD), disease)], disease);
  int stats[4] = {0};
  topkAgeSearch(tree, country, d1, d2, stats);
  int total = stats[0]+stats[1]+stats[2]+stats[3];

  heapNode * h = NULL;
  if (total != 0) {
    h = heapInsert(h, 0, stats[0]*100/total);
    h = heapInsert(h, 1, stats[1]*100/total);
    h = heapInsert(h, 2, stats[2]*100/total);
    h = heapInsert(h, 3, stats[3]*100/total);
  }
  else {
    h = heapInsert(h, 0, 0);
    h = heapInsert(h, 1, 0);
    h = heapInsert(h, 2, 0);
    h = heapInsert(h, 3, 0);
  }

  char buffer[128] = {0};
  for (int i = 0; ((i < k) && (h != NULL)); ++i) {
    memcpy(buffer+i*sizeof(int)*2, &(h->val), sizeof(int));
    memcpy(buffer+i*sizeof(int)*2+sizeof(int), &(h->key), sizeof(int));
    h = heapDelete(h);
  }
  freeHeap(h);
  int bytes = 0;
  if (k > 4)
    bytes = sizeof(int)+4*sizeof(int)*2;
  else
    bytes = sizeof(int)+k*sizeof(int)*2;

  int tag = 1;
  safeWrite(ws->ans_sock, &bytes, sizeof(int));
  safeWrite(ws->ans_sock, &tag, sizeof(int));
  safeWrite(ws->ans_sock, buffer, k*sizeof(int)*2);
  ws->success++;
}

void searchPatient(worker_space * ws, char * recID) {
  listNode * listptr = ws->list;
  Record * r = findRecord(listptr, recID);
  char buffer[256] = {0};
  int size = 6+sizeof(int);
  int tag = 3;
  if (r == NULL) {
    ws->fail++;
    memcpy(buffer, &size, sizeof(int));
    memcpy(buffer+sizeof(int), &tag, sizeof(int));
    memcpy(buffer+sizeof(int)*2, "Error", 6);
    safeWrite(ws->ans_sock, buffer, 6+sizeof(int)*2);
    return;
  }

  size = sizeof(int);
  memcpy(buffer+size, &tag, sizeof(int));
  size += sizeof(int);

  int len = (int)strlen(r->recordID)+1;
  memcpy(buffer+size, &len, sizeof(int));
  size += sizeof(int);
  memcpy(buffer+size, r->recordID, len);
  size += len;

  len = (int)strlen(r->patientFirstName)+1;
  memcpy(buffer+size, &len, sizeof(int));
  size += sizeof(int);
  memcpy(buffer+size, r->patientFirstName, len);
  size += len;

  len = (int)strlen(r->patientLastName)+1;
  memcpy(buffer+size, &len, sizeof(int));
  size += sizeof(int);
  memcpy(buffer+size, r->patientLastName, len);
  size += len;

  len = (int)strlen(r->diseaseID)+1;
  memcpy(buffer+size, &len, sizeof(int));
  size += sizeof(int);
  memcpy(buffer+size, r->diseaseID, len);
  size += len;

  len = (int)strlen(r->country)+1;
  memcpy(buffer+size, &len, sizeof(int));
  size += sizeof(int);
  memcpy(buffer+size, r->country, len);
  size += len;

  memcpy(buffer+size, &(r->age), sizeof(int));
  size += sizeof(int);

  memcpy(buffer+size, &(r->entryDate), sizeof(Date));
  size += sizeof(Date);

  memcpy(buffer+size, &(r->exitDate), sizeof(Date));
  size += sizeof(Date);

  size -= sizeof(int);
  memcpy(buffer, &size, sizeof(int));

  size += sizeof(int);
  safeWrite(ws->ans_sock, buffer, size);
  ws->success++;
}

void numAdmissions(worker_space * ws, char * virus, Date d1, Date d2, char * country) {
  int count = 0;
  int tag = 4;
  if (country == NULL) {
    int bytes = 0;
    char buffer[128] = {0};
    memcpy(buffer, &tag, sizeof(int));
    bytes += sizeof(int);
    avlNode * n = NULL;
    for (int i = 0; i < ws->n_countries; ++i) {
      n = countryTree(ws->tC->table[bIndex(*(ws->tC), ws->countries[i])], ws->countries[i]);
      int len = strlen(ws->countries[i])+1;
      memcpy(buffer+bytes, &len, sizeof(int));
      bytes += sizeof(int);
      memcpy(buffer+bytes, ws->countries[i], len);
      bytes += len;
      count = countAdmissions(n, virus, d1, d2);
      memcpy(buffer+bytes, &count, sizeof(int));
      bytes += sizeof(int);
    }
    // printf("%d %d\n", bytes, ws->ans_sock);
    safeWrite(ws->ans_sock, &bytes, sizeof(int));
    safeWrite(ws->ans_sock, buffer, bytes);
    ws->success++;
  }
  else {
    if (!countryParse(ws->countries, ws->n_countries, country)) {
      ws->fail++;
      int bytes = 6+sizeof(int);
      safeWrite(ws->ans_sock, &bytes, sizeof(int));
      safeWrite(ws->ans_sock, &tag, sizeof(int));
      safeWrite(ws->ans_sock, "ERROR", 6);
      return;
    }
    avlNode * n = countryTree(ws->tC->table[bIndex(*(ws->tC), country)], country);
    count = countAdmissions(n, virus, d1, d2);
    int len = strlen(country)+1;
    int bytes = len + sizeof(int)*3;
    safeWrite(ws->ans_sock, &bytes, sizeof(int));
    safeWrite(ws->ans_sock, &tag, sizeof(int));
    safeWrite(ws->ans_sock, &len, sizeof(int));
    safeWrite(ws->ans_sock, country, len);
    safeWrite(ws->ans_sock, &count, sizeof(int));
    ws->success++;
  }
}

void numDischarges(worker_space * ws, char * virus, Date d1, Date d2, char * country) {
  int count = 0;
  int tag = 5;
  if (country == NULL) {
    int bytes = 0;
    char buffer[128] = {0};
    memcpy(buffer+bytes, &tag, sizeof(int));
    bytes += sizeof(int);
    avlNode * n = NULL;
    for (int i = 0; i < ws->n_countries; ++i) {
      n = countryTree(ws->tC->table[bIndex(*(ws->tC), ws->countries[i])], ws->countries[i]);
      int len = strlen(ws->countries[i])+1;
      memcpy(buffer+bytes, &len, sizeof(int));
      bytes += sizeof(int);
      memcpy(buffer+bytes, ws->countries[i], len);
      bytes += len;
      count = countDischarges(n, virus, d1, d2);
      memcpy(buffer+bytes, &count, sizeof(int));
      bytes += sizeof(int);
    }
    safeWrite(ws->ans_sock, &bytes, sizeof(int));
    safeWrite(ws->ans_sock, buffer, bytes);
    ws->success++;
  }
  else {
    if (!countryParse(ws->countries, ws->n_countries, country)) {
      int bytes = 6+sizeof(int);
      safeWrite(ws->ans_sock, &bytes, sizeof(int));
      safeWrite(ws->ans_sock, &tag, sizeof(int));
      safeWrite(ws->ans_sock, "ERROR", 6);
      ws->fail++;
      return;
    }
    avlNode * n = countryTree(ws->tC->table[bIndex(*(ws->tC), country)], country);
    count = countDischarges(n, virus, d1, d2);
    int len = strlen(country)+1;
    int bytes = len + sizeof(int)*2;
    safeWrite(ws->ans_sock, &bytes, sizeof(int));
    safeWrite(ws->ans_sock, &tag, sizeof(int));
    safeWrite(ws->ans_sock, &len, sizeof(int));
    safeWrite(ws->ans_sock, country, len);
    safeWrite(ws->ans_sock, &count, sizeof(int));
    ws->success++;
  }
}

char ** get_countries(char * path, int low, int high) {
  char ** countries = malloc((high-low+1)*sizeof(char *));
  DIR * dirp = {0};
  struct dirent * dir = {0};
  dirp = opendir(path);
  if (dirp >= 0) {
    for (int i = 0; i < low-1; ++i) {
      dir = readdir(dirp);
      if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
        --i;
        continue;
    }
    /*if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
      dir = readdir(dirp);*/
  }
  else {
    perror("opendir");
    exit(1);
  }

  for (int i = 0; i < (high-low+1); ++i) {
    dir = readdir(dirp);
    if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) {
      --i;
      continue;
    }
    countries[i] = malloc(strlen(dir->d_name)+1);
    strcpy(countries[i], dir->d_name);
  }
  closedir(dirp);
  return countries;
}

char ** get_date_files(char * path, int * file_num) {
  int dir_count = 0;
  DIR * dirp = {0};
  struct dirent * dir = {0};
  dirp = opendir(path);
  if (dirp < 0) {
    perror("opendir");
    exit(1);
  }
  *file_num = file_counter(path);
  char ** date_files = malloc((*file_num)*sizeof(char *));
  for (int i = 0; i < *file_num; ++i)
    date_files[i] = malloc(11);

  for (int i = 0; i < (*file_num); ++i) {
    dir = readdir(dirp);
    if (!strcmp (dir->d_name, ".") || !strcmp (dir->d_name, "..")) {
      --i;
      continue;
    }
    strcpy(date_files[i], dir->d_name);
  }
  closedir(dirp);
  return date_files;
}

void merge(char ** dates, int l, int m, int r) {
  int i, j, k;
  int n1 = m-l+1;
  int n2 = r-m;

  char ** L = malloc(n1*sizeof(char *));
  for (int c = 0; c < n1; ++c)
    L[c] = malloc(11);
  char ** R = malloc(n2*sizeof(char *));
  for (int c = 0; c < n2; ++c)
    R[c] = malloc(11);

  for (i = 0; i < n1; i++)
    strncpy(L[i], dates[l+i], 10);
  for (j = 0; j < n2; j++)
    strncpy(R[j], dates[m+1+j], 10);
  i = 0;
  j = 0;
  k = l;
  while (i < n1 && j < n2) {
    Date d1 = {0};
    Date d2 = {0};
    stringToDate(L[i], &d1);
    stringToDate(R[j], &d2);
    if (dateCompare(d1, d2) <= 0) {
      strncpy(dates[k], L[i], 10);
      i++;
    }
    else {
      strncpy(dates[k], R[j], 10);
      j++;
    }
    k++;
  }

  while (i < n1) {
    strncpy(dates[k], L[i], 10);
    i++;
    k++;
  }
  while (j < n2) {
    strncpy(dates[k], R[j], 10);
    j++;
    k++;
  }

  for (int i = 0; i < n1; ++i)
    free(L[i]);
  free(L);
  for (int i = 0; i < n2; ++i)
    free(R[i]);
  free(R);
}

void sort_date_files(char ** dates, int l, int r) {
  if (l < r) {
    int m = (l+r)/2;
    sort_date_files(dates, l, m);
    sort_date_files(dates, m+1, r);
    merge(dates, l, m, r);
  }

}
void set_up(char * path, listNode ** l, char * country, char * date) {

  char buffer[128] = {0};
  char * eofptr = NULL;

  Record r = {0};
  FILE * f = fopen(path, "r");
  eofptr = fgets(buffer, 128, f);
  int val = 0;

  for (int i = 0; eofptr != NULL; ++i) {
    if (!(val = makeRecord(l, &r, buffer, country, date)))
      listInsert(l, r);
    else if (val < 0)
      printf("Error2\n");
    eofptr = fgets(buffer, 128, f);
  }
  fclose(f);
}

int end_of_work(char ** countries, int n, int total, int succ, int fail) {
  char path[64] = {0};
  strcpy(path, "./log_files/log_file.");
  sprintf(path+21, "%d", getpid());

  FILE * f = fopen(path, "w");
  if (f == NULL) {
    perror("Error creating log_file");
    return -1;
  }
  for (int i = 0; i < n; ++i)
    fprintf(f, "%s\n", countries[i]);
  fprintf(f,"TOTAL %d\nSUCCESS %d\nFAIL %d\n", total, succ, fail);
  fclose(f);
  return 0;
}

void worker_exit(worker_space * ws) {
  flag1 = 0;
  end_of_work(ws->countries, ws->n_countries, ws->total, ws->success, ws->fail);

  /*freeHashTable(ws->tC);
  freeHashTable(ws->tD);
  listFree(ws->list);

  free(ws->pipe_buff);

  close(ws->ans_sock[0]);
  close(ws->fds[1]);

  free(ws->viruses);

  for (int j = 0; j < ws->n_vir; ++j)
      free(ws->stats[j]);
  free(ws->stats);

  for (int i = 0; i < ws->n_countries; ++i)
    free(ws->countries[i]);
  free(ws->countries);
  free(ws->file_num);

  free(ws);*/
  // printf("Worker with pid %d ready to leave!\n", getpid());
}
