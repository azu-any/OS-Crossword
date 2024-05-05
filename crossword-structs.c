// Azuany Mila Ceron - 177068
// Final Project - Operating Systems
// Spring 2024
// Last modified: 2024/04/21
// This program creates a crossword puzzle.

#include "thread_functions.c"
#include "signal_handlers.c"
#include "utils.c"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {

	int complete = 0;
	char word_number_char;
	int word_number;
	int word_selected = 0;

	initializeWords();

	// Wait for signals
	signal(SIGALRM, change_word_handler);
	signal(SIGINT, sig_handler_sigint);

	// Initialize crosswords
	updateCrossword(my_words, crossword, solvedCrossword);

	printf("*** Welcome ***\n");
	printf("This is \"La casa de hojas\"\n");

	pthread_t thread;
	pthread_create(&thread, NULL, increase_counter, NULL);

	printCrossword(solvedCrossword);
	while(complete == 0) {
		printCrossword(crossword);
		//updateCrossword(my_words, crossword, solvedCrossword);
		printClues(my_words);

		while(word_selected == 0) {
			printf("\nWhich number of word would you like to guess?\n");
			word_number_char = getchar();
			fflush(stdin);

			word_number = word_number_char - '0';

			if(word_number <= 6 && word_number >=1){

				word_number--;

				char *definition = my_words[word_number].word[my_words[word_number].index].definition;
				char *word = my_words[word_number].word[my_words[word_number].index].word;
				int row = my_words[word_number].word[my_words[word_number].index].row;
				int col = my_words[word_number].word[my_words[word_number].index].col;
				char direction = my_words[word_number].direction;
				int size = strlen(word);

				printf("\nGuess the word %d with %d letters: %s\n", word_number + 1, size, definition);

				int guessed_letters = 0;
				for(int i=0; i<size; i++) {
					if(isdigit(crossword[row][col+i])) {
						printf("Letter %d: ", i);
						char my_char = getchar();
						fflush(stdin);
						if(direction == 'H') {
							if(my_char == solvedCrossword[row][col+i]){
								guessed_letters++;
								crossword[row][col+i] = my_char;
								printCrossword(crossword);
							}
						}
					} else {
						guessed_letters++;
					}
				}

				if(guessed_letters == size){
					my_words[word_number].locked = 1;
				}

				/*for(int i=0; i<size; i++) {

					printf("Letter %d", i);
					char my_char = getchar();
					fflush(stdin);

					if (my_char == word[i]) {
						for(int j=0; j<size; j++){
							if(j==i){
								printf("%c ", my_char);
							} else {
								printf("_ ");
							}
						}
					}
					}*/


			} else {
				printf("\nWrong answer. Select a number from 1 to 6\n");
			}
		}
	}

	return 0;
}
