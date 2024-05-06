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
#define WAIT_TIME 45

// GLOBAL VARIABLES

char crossword[ROWS][COLS];
char solvedCrossword[ROWS][COLS];
struct words my_words[6];
pthread_mutex_t mutex;
pthread_cond_t cond_consumer, cond_producer;
int complete = 0, guessed_letters = 0, k = 0;

#define NO_ITEMS 10
int buffer = 0;


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


void* producer(void* arg)
{
    for ( int i=0; i<NO_ITEMS; i++ )
    {
        pthread_mutex_lock( &mutex );
        while( buffer != 0 ) // Esperar hasta que el buffer esté libre
        {
            // Libera el mutex y espera a que se cumpla la condición (que llegue la señal)
            pthread_cond_wait( &cond_producer, &mutex );
        }
        buffer = 1;
        printf("Productor: escribí un dato en el buffer.\n");

        // Notifica que hay elementos en el buffer
        pthread_cond_signal( &cond_consumer );
        pthread_mutex_unlock( &mutex );
    }

    pthread_exit( NULL );
}

void* consumer(void* arg)
{
    for ( int i=0; i<NO_ITEMS; i++ )
    {
        pthread_mutex_lock( &mutex );
        // ---------- Región crítica
        while (buffer == 0) // Esperar hasta que haya información en el buffer
        {
            // Libera el mutex y espera a que se cumpla la condición (que llegue la señal)
            pthread_cond_wait( &cond_consumer, &mutex );
        }
        printf("Consumidor: leí un dato del buffer.\n");
        buffer = 0;

        // Avisa que el buffer está limpio.
        pthread_cond_signal( &cond_producer );
        pthread_mutex_unlock( &mutex );

    }

    pthread_exit(NULL);
}


// UTILS FUNCTIONS

void printWelcome() {
	printf("*** WELCOME ***\n");
	printf("\nThis is \"La casa de hojas\"\n");
	printf("It's your turn to play!");
}


void printRules() {
	printf("\n***Rules***\n");
	printf("Read carefully the rules of the game before playing\n");


}


void printClues(struct words words[6]) {
	printf("\nHORIZONTAL\n");
	printf("1. %s\n", words[0].word[words[0].index].definition);
	printf("2. %s\n", words[1].word[words[1].index].definition);
	printf("3. %s\n", words[2].word[words[2].index].definition);

	printf("\nVERTICAL\n");
	printf("4. %s\n", words[3].word[words[3].index].definition);
	printf("5. %s\n", words[4].word[words[4].index].definition);
	printf("6. %s\n", words[5].word[words[5].index].definition);
}


void printCrossword(char crossword[ROWS][COLS]) {
	printf("\nCROSSWORD\n");
	for (int i = 0; i < ROWS+1 ; i++) {
		for (int j = 0; j < COLS+1; j++) {
			if (i == 0 && j == 0) {
				printf("\t");
			} else if (i == 0) {
				printf("%d\t", j);
			} else if (j == 0) {
				printf("%d\t", i);
			} else {
				printf("%c\t", crossword[i - 1][j - 1]);
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
					crossword[row][col + i] = hide_char;
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
					crossword[row + i][col] = hide_char;
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


// THREAD FUNCTIONS

int counter = 0;
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


// SIGNAL HANDLERS

void change_word_handler(int signum) {
	printf("\nTe tardaste mucho :(\nUna palabra cambiará\n");

	kill(getpid(), SIGUSR1);

	if (complete == 1){
		return;
	}

	int flag = 0;
	while(flag == 0) {

		int number = rand() % (NO_WORDS) + 0;
		printf("%d", number);

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

	printCrossword(crossword);
	printClues(my_words);
}


// In case, user enters CTRL + C
void sig_handler_sigint(int signum) {

	while (1) {
		printf("Are you sure you want to exit? [Y/n]");
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


void sig_handler_stop_reading(int signum) {
	k = 10;
	guessed_letters = 0;
}


void sig_handler_clear_console(int signum) {
	execl(const char *path, const char *arg0, ...)
}
