// Azuany Mila Ceron - 177068
// Final Project - Operating Systems
// Spring 2024
// Last modified: 2024/04/21
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
#include <unistd.h>

int main() {

	pid_t pid_A;

	/*if ((pid_A = fork()) == 0) {

	}*/

	char word_number_char;
	int word_number;
	int word_selected = 0;
	int guessed_words = 0;

	// Initialize crosswords
	initializeWords();
	updateCrossword(my_words, crossword, solvedCrossword);

	// Wait for signals
	signal(SIGALRM, change_word_handler);
	signal(SIGINT, sig_handler_sigint);
	signal(SIGUSR1, sig_handler_stop_reading);
	signal(SIGUSR2, sig_handler_clear_console);

	// start counter to change words
	pthread_t thread;
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

			char *definition = my_words[word_number].word[my_words[word_number].index].definition;
			char *word = my_words[word_number].word[my_words[word_number].index].word;
			int row = my_words[word_number].word[my_words[word_number].index].row;
			int col = my_words[word_number].word[my_words[word_number].index].col;
			char direction = my_words[word_number].direction;
			int size = strlen(word);

			printf("\nGuess the word %d with %d letters: %s\n", word_number + 1, size, definition);

			guessed_letters = 0;
			k = 0;
			for(int k=0; k<size; k++) {
				if(direction == 'H') {
					if(isdigit(crossword[row][col+k])) {
						printf("Letter %d: ", k+1);
						char my_char = getchar();
						fflush(stdin);

						if(my_char == solvedCrossword[row][col+k]){
							guessed_letters++;
							crossword[row][col+k] = my_char;
							printCrossword(crossword);
						}

					} else {
						guessed_letters++;
					}
				} else {

					if(isdigit(crossword[row+k][col])) {
						printf("Letter %d: ", k+1);
						char my_char = getchar();
						fflush(stdin);

						if(my_char == solvedCrossword[row+k][col]) {
								guessed_letters++;
								crossword[row+k][col] = my_char;
								printCrossword(crossword);
						}
					} else {
						guessed_letters++;
					}
				}
			}

			if(guessed_letters == size){
				my_words[word_number].locked = 1;
				guessed_words++;

				kill(getpid(), SIGUSR2);

				printf("\nYou guessed the word!\n");
				printCrossword(crossword):

				if (guessed_words == NO_WORDS) {
					complete = 1;
					break;
				}
			}

		} else {
			printf("\nWrong answer. Select a number from 1 to 6\n");
		}
	}

	if(complete == 1) {
		printf("\nCongratualtions you finished the crossword\n");
	}

	return 0;
}
