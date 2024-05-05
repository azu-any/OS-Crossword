// Azuany Mila Ceron - 177068
// Final Project - Operating Systems
// Spring 2024
// Last modified: 2024/04/21
// This program creates a crossword puzzle.

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
/*
int counter = 0;
void *increase_counter(void *args) {
  while (1) {
    counter++;
    sleep(1);

    if (counter % 35 == 0) {
      alarm(1);
    } else if (counter % 45 == 0) {
      alarm(1);
    } else if (counter % 55 == 0) {
      alarm(1);
    }
  }
  pthread_exit(NULL);
}


void change_word_handler(int signum) {
	printf("\nTe tardaste mucho :(\nUna palabra cambiará\n");

	int flag = 0;
	while(flag == 0) {

		int number = rand() % (5 + 1 - 0) + 0;
		printf("%d", number);

		if (my_words[number].locked == 0) {

		}
	}
}
*/
