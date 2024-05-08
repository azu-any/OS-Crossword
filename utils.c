// Azuany Mila Ceron - 177068
// Final Project - Operating Systems
// Spring 2024
// Last modified: 2024/05/05
// Functions for crossword puzzle.


#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/_pthread/_pthread_t.h>
#include <sys/_types/_null.h>
#include <sys/_types/_pid_t.h>
#include <sys/signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/types.h>
#include "errors.c"


// STRUCT DEFINITIONS

typedef struct word {
	char word[10];
	int row;
	int col;
	char definition[100];
} word;


typedef struct words {
	int index;
	int locked;
	char direction;
	word word[3];
} words;


// CONSTANTS

#define ROWS 15
#define COLS 10
#define NO_WORDS 6
#define WAIT_TIME 30
#define NO_RULES 12


// GLOBAL VARIABLES

char crossword[ROWS][COLS];
char solvedCrossword[ROWS][COLS];
struct words my_words[6];
pthread_t thread[2];
pthread_mutex_t mutex;
pthread_cond_t cond_readRule, cond_checkRule;
pthread_cond_t cond_readLetter, cond_checkLetter;
int complete = 0, guessed_letters = 0, counter = 0;
int rule = 0, rule_counter = 0, buffer = 0;
int scan = 1;
char my_char;


// FUNCTION PROTOTYPES

void *increase_counter(void *args);
void *defineWord(void *arg);
void initializeWords();
void printClues(struct words words[6]);
void printCrossword(char crossword[ROWS][COLS]);
void writeWord(words words, char crossword[ROWS][COLS], int hide);
void updateCrossword(words words[], char crossword[ROWS][COLS], char solvedCrossword[ROWS][COLS]);
void sig_handler_sigint(int signum);
void change_word_handler(int signum);
void* readRule(void* arg);
void* checkRule(void* arg);
void* readLetter(void* arg);
void* checkLetter(void* arg);
void sig_handler_stop(int signum);


// UTILS FUNCTIONS

void printWelcome() {

	system("clear"); // to clear screen

	printf("\n***************\n");
	printf("*** WELCOME ***\n");
	printf("***************\n");
	printf("\nThis is \"La casa de hojas\"\n");
	printf("\nIt's your turn to play!\n");
	printf("\nPress enter to continue\n");

	getchar();
	fflush(stdin);

}


void printCongratulations() {

	printf("\n***********************\n");
	printf("*** CONGRATULATIONS ***\n");
	printf("***********************\n");
	printf("You finished the crossword!\n");

	printCrossword(crossword);

}


void printRules() {

	//pthread_t thread[2];
	pthread_cond_init( &cond_readRule, 0 );
    pthread_cond_init( &cond_checkRule, 0 );

    system("clear"); // to clear screen

	printf("\n*** RULES ***\n");
	printf("Read carefully the rules of the game before playing\n");
	printf("Press enter to continue and read all rules\n");
	getchar();
	fflush(stdin);

	pthread_create(&thread[0], NULL, readRule, NULL );
	pthread_create(&thread[1], NULL, checkRule, NULL );

	pthread_join(thread[0], NULL);
    pthread_join(thread[1], NULL);

    pthread_mutex_destroy( &mutex );
    pthread_cond_destroy( &cond_readRule );
    pthread_cond_destroy( &cond_checkRule );

    system("clear");
    printf("\nGOOD LUCK! Enjoy the game ;)\n");
}


void printClues(struct words words[6]) {

	printf("\nHORIZONTAL\n");
	for(int i=0; i<3; i++) {
		printf("%d. %s\n", i+1, words[i].word[words[i].index].definition);
	}

	printf("\nVERTICAL\n");
	for(int i=3; i<6; i++) {
		printf("%d. %s\n", i+1, words[i].word[words[i].index].definition);
	}
}


void printCrossword(char crossword[ROWS][COLS]) {

	printf("\nCROSSWORD\n");
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLS; j++) {
			if(crossword[i][j] == '1'){
				printf("\033[41m%c\t\033[0m", crossword[i][j]);
			} else if(crossword[i][j] == '2'){
				printf("\033[42m%c\t\033[0m", crossword[i][j]);
			} else if(crossword[i][j] == '3'){
				printf("\033[43m%c\t\033[0m", crossword[i][j]);
			} else if(crossword[i][j] == '4'){
				printf("\033[44m%c\t\033[0m", crossword[i][j]);
			} else if(crossword[i][j] == '5'){
				printf("\033[45m%c\t\033[0m", crossword[i][j]);
			} else if(crossword[i][j] == '6'){
				printf("\033[46m%c\t\033[0m", crossword[i][j]);
			} else if(crossword[i][j] == '*'){
				printf("\033[40m%c\t\033[0m", crossword[i][j]);
			} else if(crossword[i][j] != '-'){
				printf("\033[47m\033[30m%c\t\033[37m", crossword[i][j]);
			} else {
				printf("\033[0m%c\t", crossword[i][j]);
			}

		}
		printf("\n");
	}
	printf("\n");

}


void writeWord(words words, char crossword[ROWS][COLS], int hide) {

	int i = 0;
	int row = words.word[words.index].row;
	int col = words.word[words.index].col;
	char direction = words.direction;
	char hide_char = hide + '0';
	char word[10];
	strncpy(word, words.word[words.index].word, 10);


	if (direction == 'H') {
		while (word[i] != '\0') {
			if (hide == 0) {
	    		crossword[row][col + i] = word[i];
			} else {
				if(!isalpha(crossword[row][col+i])){
					if(isdigit(crossword[row][col+i])){
						crossword[row][col + i] = '*';
					} else {
						crossword[row][col + i] = hide_char;
					}
				}
			}
		    i++;
	    }
	} else {
	    while (word[i] != '\0') {
			if (hide == 0) {
		    	crossword[row + i][col] = word[i];
			} else {
				if(!isalpha(crossword[row + i][col])){
					if(isdigit(crossword[row+i][col])){
						crossword[row+i][col] = '*';
					} else {
						crossword[row + i][col] = hide_char;
					}
				}
			}
		    i++;
	    }
	}
}


void updateCrossword(words words[], char crossword[ROWS][COLS], char solvedCrossword[ROWS][COLS]) {

	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLS; j++) {
			crossword[i][j] = '-';
			solvedCrossword[i][j] = '-';
		}
	}

	for (int i = 0; i < NO_WORDS; i++) {
	    writeWord(words[i], solvedCrossword, 0);
		if (words[i].locked == 0) {
			writeWord(words[i], crossword, i+1);
		} else {
			writeWord(words[i], crossword, 0);
		}
	}
}


void initializeWords() {
    pthread_t threads[NO_WORDS];
    pthread_mutex_init( &mutex, 0 );

    int indices[NO_WORDS];
    for (int i = 0; i < 6; i++) {
        indices[i] = i;
        pthread_create(&threads[i], NULL, defineWord, &indices[i]);
    }

    for (int i = 0; i < 6; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy( &mutex );
    return;
}


void readInput(int index){

	//pthread_t thread[2];
	pthread_cond_init( &cond_readLetter, 0 );
    pthread_cond_init( &cond_checkLetter, 0 );

    system("clear"); // to clear screen
    char *word = my_words[index].word[my_words[index].index].word;
    int size = strlen(word);

    printCrossword(crossword);

	pthread_create(&thread[0], NULL, readLetter, (void *)&index);
	pthread_create(&thread[1], NULL, checkLetter, (void *)&index );

	pthread_join(thread[0], NULL);
    pthread_join(thread[1], NULL);

    pthread_mutex_destroy( &mutex );
    pthread_cond_destroy( &cond_readLetter );
    pthread_cond_destroy( &cond_checkLetter );

}


// THREAD FUNCTIONS

void* readLetter(void* arg) {

	signal(SIGINT, sig_handler_sigint);

	int index = *(int *)arg;
	char *word = my_words[index].word[my_words[index].index].word;
	int row = my_words[index].word[my_words[index].index].row;
	int col = my_words[index].word[my_words[index].index].col;
	char direction = my_words[index].direction;
	char *definition = my_words[index].word[my_words[index].index].definition;
	int size = strlen(word);
	buffer = 0;

    for ( int i=0; i<size; i++ ) {

        pthread_mutex_lock( &mutex );
        while( buffer != 0 ) {
            pthread_cond_wait( &cond_readLetter, &mutex );
        }
        if(scan == 1) {
	        if(direction == 'H') {
				if(!(isdigit(crossword[row][col+i]) || crossword[row][col+i] == '*')) {
					buffer = 1;
					continue;
				}
			} else {
				if(!(isdigit(crossword[row+i][col]) || crossword[row+i][col] == '*')) {
					buffer = 1;
					continue;
				}
			}

			printf("\nGuess the word %d with %d letters: %s\n", index + 1, size, definition);
	        printf("Letter %d: ", i+1);
	        while ((my_char = getchar()) == '\n') {
	       		if(scan == 0){
	         		buffer = 1;
					pthread_cond_signal( &cond_checkLetter );
				    pthread_mutex_unlock( &mutex );
					pthread_exit( NULL );
	        	}

	        	printf("Try again. Letter cannot be empty\n");

	        }
			fflush(stdin);
	        buffer = 1;
	        pthread_cond_signal( &cond_checkLetter );
	        pthread_mutex_unlock( &mutex );

    	} else {
     		buffer = 1;
	        pthread_cond_signal( &cond_checkLetter );
	        pthread_mutex_unlock( &mutex );
       		break;

     	}
    }

    pthread_exit( NULL );

}


void* checkLetter(void* arg) {

	int index = *(int *)arg;
	signal(SIGINT, sig_handler_sigint);

	char *word = my_words[index].word[my_words[index].index].word;
	int row = my_words[index].word[my_words[index].index].row;
	int col = my_words[index].word[my_words[index].index].col;
	char direction = my_words[index].direction;
	int size = strlen(word);

    for ( int i=0; i<size; i++ ) {
        pthread_mutex_lock( &mutex );
        while (buffer == 0) {
            pthread_cond_wait( &cond_checkLetter, &mutex );
        }
        if(scan == 1) {

        	my_char = tolower(my_char);

	        if(direction == 'H') {
				if(isalpha(crossword[row][col+i])) {
					guessed_letters++;
					 buffer = 0;
					continue;
				}
			} else {
				if(isalpha(crossword[row+i][col])) {
					guessed_letters++;
					 buffer = 0;
					continue;
				}
			}

			if(direction == 'H') {
				if(isdigit(crossword[row][col+i]) || crossword[row][col+i] == '*') {
					if(my_char == solvedCrossword[row][col+i]){
						guessed_letters++;
						crossword[row][col+i] = my_char;
						system("clear");
						printCrossword(crossword);
					}
				}
			} else {
				if(isdigit(crossword[row+i][col]) || crossword[row+i][col] == '*') {
					if(my_char == solvedCrossword[row+i][col]) {
						guessed_letters++;
						crossword[row+i][col] = my_char;
						system("clear");
						printCrossword(crossword);
					}
				}
			}
			buffer = 0;
	        pthread_cond_signal( &cond_readLetter );
	        pthread_mutex_unlock( &mutex );

        } else {
        	buffer = 0;
	        pthread_cond_signal( &cond_readLetter );
	        pthread_mutex_unlock( &mutex );
        	break;
        }
    }
    pthread_exit(NULL);
}


void *increase_counter(void *args) {

	while (1) {
		counter++;
		sleep(1);

		if (counter % WAIT_TIME == 0)
			alarm(1);
	}
	pthread_exit(NULL);
}


void *defineWord(void *arg) {

	pthread_mutex_lock( &mutex );

    int i = *(int *)arg;
    char *words_file[NO_WORDS] = { "horizontal/first.txt",
	                            	"horizontal/second.txt",
				                    "horizontal/third.txt",
				                    "vertical/first.txt",
				                    "vertical/second.txt",
				                    "vertical/third.txt"
				                };

    my_words[i].index = rand() % (2 + 1 - 0) + 0; // random word to appear
    my_words[i].locked = 0;
    my_words[i].direction = (words_file[i][0] == 'h') ? 'H' : 'V';

    // open and read file
    int fd = open(words_file[i], 0);
    check_error(fd);
    char buffer[250];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    check_error(bytes_read);
    buffer[bytes_read] = '\0';

	sscanf(buffer, "%s %d %d %99[^\n] %s %d %d %99[^\n] %s %d %d %99[^\n]",
		my_words[i].word[0].word, &my_words[i].word[0].row, &my_words[i].word[0].col, my_words[i].word[0].definition,
		my_words[i].word[1].word, &my_words[i].word[1].row, &my_words[i].word[1].col, my_words[i].word[1].definition,
		my_words[i].word[2].word, &my_words[i].word[2].row, &my_words[i].word[2].col, my_words[i].word[2].definition);


    close(fd);
    pthread_mutex_unlock( &mutex );
	pthread_exit(NULL);
}


void* readRule(void* arg) {
	char *rules[NO_RULES] =
		{
			"1. In the crossword you will see a board with '-', '*', and numbers.",
			"2. Each number represents a letter of a word.",
			"3. A word is made up by the same number.",
			"4. An intersection between words is written with '*'.",
			"5. At the end of the crossword, the clues of the words are written.",
			"6. You should enter a number between 1 and 6 according to the word you want to guess.",
			"7. Once the word is selected, you should enter letter by letter the word",
			"8. Look at the number of letter, since some letters will be already guessed, it won't be necessary to enter them again.",
			"9. If you have guessed the right letter, the crossword, will update.",
			"10. If your answer is worng, you'll continue guessing the other letters.",
			"11. Look out for the time! After some seconds, words you haven't completely guessed will start to change.",
			"12. Words can change in meaning and length, but the intersections will remain.",
		};

    for ( int i=0; i<NO_RULES; i++ ) {

        pthread_mutex_lock( &mutex );
        while( rule != 0 ) {
            pthread_cond_wait( &cond_readRule, &mutex );
        }

        printf("%s", rules[rule_counter]);
        rule_counter++;
        rule = 1;

        pthread_cond_signal( &cond_checkRule );
        pthread_mutex_unlock( &mutex );
    }

    pthread_exit( NULL );

}


void* checkRule(void* arg) {

    for ( int i=0; i<NO_RULES; i++ ) {
        pthread_mutex_lock( &mutex );
        while (rule == 0) {
            pthread_cond_wait( &cond_checkRule, &mutex );
        }
        getchar();
        fflush(stdin);
        rule = 0;

        pthread_cond_signal( &cond_readRule );
        pthread_mutex_unlock( &mutex );

    }
    pthread_exit(NULL);

}


// SIGNAL HANDLERS

void change_word_handler(int signum) {

	scan = guessed_letters = 0;
	printf("\nYou took too longue :(\nA word will change\n");


	if (complete == 1){
		return;
	}

	int flag = 0;
	while(flag == 0) {

		int number = rand() % (NO_WORDS) + 0;

		if (my_words[number].locked == 0) {
			flag = 1;
			if(my_words[number].index<2) {
				 my_words[number].index++;
			} else {
				my_words[number].index = 0;
			}
			updateCrossword(my_words, crossword, solvedCrossword);
		}
	}

	printf("\n Click ENTER to continue \n");

}


// In case, user enters CTRL + C
void sig_handler_sigint(int signum) {

	scan = guessed_letters = 0;

	while (1) {
		printf("\nAre you sure you want to exit? [Y/n] ");
		char confirm = getchar();
		fflush(stdin);

		if (confirm == 'Y' || confirm == 'y') {
			printf("\nSee you later :(\n");
			exit(0);
		} else if (confirm == 'N' || confirm == 'n') {
			printf("\nLet's continuing playing!\n");
			return;
		}

		printf("\nYour answer could not be processed. Please try again\n");
	}
}
