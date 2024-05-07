// Azuany Mila Ceron - 177068
// Final Project - Operating Systems
// Spring 2024
// Last modified: 2024/05/06
// This program creates a crossword puzzle.

#include "utils.c"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_pid_t.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {

	pid_t pid_A, pid_B;

	if ((pid_A = fork()) == 0) {

		if ((pid_B = fork()) == 0) {
			printWelcome();
			return 0;
		}

		waitpid(pid_B, NULL, 0);
		printRules();
		return 0;
	}

	char word_number_char;
	int word_number;
	int word_selected = 0, guessed_words = 0;
	pthread_t thread;

	// initialize crosswords
	initializeWords();
	updateCrossword(my_words, crossword, solvedCrossword);

	// wait for signals
	signal(SIGALRM, change_word_handler);
	signal(SIGINT, sig_handler_sigint);
	//signal(SIGUSR1, sig_handler_stop_reading);
    //signal(SIGUSR2, sig_handler_stop);

	// wait for user to read all rules
	waitpid(pid_A, NULL, 0);
	// start counter to change words
	pthread_create(&thread, NULL, increase_counter, NULL);

	while(1) {

		printCrossword(crossword);
		printClues(my_words);

		printf("\nWhich number of word would you like to guess?\n");
		word_number_char = getchar();
		fflush(stdin);

		word_number = word_number_char - '0';

		if(word_number <= NO_WORDS && word_number >=1){

			word_number--;

			if(my_words[word_number].locked != 0){
				printf("\nOh no! You have already guessed that word. Try another one!\n");
				continue;
			}

			char *word = my_words[word_number].word[my_words[word_number].index].word;
			int size = strlen(word);
			scan = 1;
			guessed_letters = 0;

			readInput(word_number); // call thread functions of for reading and checking letters

			if(guessed_letters == size){
				my_words[word_number].locked = 1;
				guessed_words++;

				system("clear");

				printf("\nYou guessed the word!\n");

				if (guessed_words == NO_WORDS) {
					complete = 1;
					break;
				}
			}

		} else {
			printf("\nWrong answer. Select a number from 1 to 6\n");
		}
	}

	printCongratulations();

	return 0;
}
