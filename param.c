#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*Getting the values from command line*/
void Param(char ** argv, char * input, int * numOfWorkers, char * bufferSize, char * serverPort, char * serverIP) {
  char flags[5] = {0};

  for (int i = 1; i < 11; i += 2) {
    if (!flags[0] && !strcmp(argv[i], "-i")) {
      flags[0] = 1;
      strcpy(input, argv[i+1]);
    }
    else if (!flags[1] && !strcmp(argv[i], "-w")) {
      flags[1] = 1;
      *numOfWorkers = atoi(argv[i+1]);
    }
    else if (!flags[2] && !strcmp(argv[i], "-b")) {
      flags[2] = 1;
      strcpy(bufferSize, argv[i+1]);
    }
    else if (!flags[3] && !strcmp(argv[i], "-s")) {
      flags[3] = 1;
      strcpy(serverIP, argv[i+1]);
    }
    else if (!flags[4] && !strcmp(argv[i], "-p")) {
      flags[4] = 1;
      strcpy(serverPort, argv[i+1]);
    }
    else {
      printf("To run properly:\n./diseaseAggregator -i input_dir");
      printf("–w numberOfWorkers -b bufferSize -s serverIP -p serverPort\n");
      exit(1);
    }
  }
  if ( !flags[0] | !flags[1] | !flags[2] | !flags[3] | !flags[4]) {
    printf("To run properly:\n./diseaseAggregator -i input_dir");
    printf("–w numberOfWorkers -b bufferSize -s serverIP -p serverPort\n");
    exit(1);
  }
}
