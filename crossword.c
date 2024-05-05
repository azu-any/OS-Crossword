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

struct word {
  int number;
  char word[10];
  int row;
  int col;
  char direction;
  char definition[50];
  int hide;
};

struct words {
  struct word words[6];
};

char word_first_horizontal[2][10] = {"fork", "12"};    // walk - speak
char word_first_vertical[2][10] = {"kill", "15"};      // skill - skull
char word_second_horizontal[2][10] = {"signal", "40"}; // normal - formal
char word_second_vertical[2][10] = {"alarm", "44"};    // lava - java
char word_third_horizontal[2][10] = {"wait", "63"};    // waitpid - await
char word_third_vertical[2][10] = {"thread", "66"};    // talk - thunder

char words[6][2][10] = {{"fork", "12"},  {"kill", "15"}, {"signal", "40"},
                        {"alarm", "44"}, {"wait", "63"}, {"thread", "66"}};

void writeWord(char word[2][10], char direction, char crossword[15][10]) {
  int i = 0;
  int row = (int)word[1][0] - '0';
  int col = (int)word[1][1] - '0';

  if (direction == 'H') {
    while (word[0][i] != '\0') {
      crossword[row][col + i] = word[0][i];
      i++;
    }

  } else {
    while (word[0][i] != '\0') {
      crossword[row + i][col] = word[0][i];
      i++;
    }
  }
}

void hideWord(char word[2][10], char direction, char crossword[15][10],
              int hide) {
  int i = 0;
  int row = (int)word[1][0] - '0';
  int col = (int)word[1][1] - '0';

  char hide_char = hide + 1 + '0';

  if (direction == 'H') {
    while (word[0][i] != '\0') {
      crossword[row][col + i] = hide_char;
      i++;
    }
  } else {
    while (word[0][i] != '\0') {
      crossword[row + i][col] = hide_char;
      i++;
    }
  }
}

// Create the  crossword puzzle
void updateCrossword(char crossword[15][10], int hide) {
  for (int i = 0; i < 15; i++) {
    for (int j = 0; j < 10; j++) {
      crossword[i][j] = '-';
    }
  }

  if (hide == 0) {
    for (int i = 0; i < 6; i++) {
      if (i % 2 == 0) {
        writeWord(words[i], 'H', crossword);
      } else {
        writeWord(words[i], 'V', crossword);
      }
    }
  } else {
    for (int i = 0; i < 6; i++) {
      if (i % 2 == 0) {
        hideWord(words[i], 'H', crossword, i);
      } else {
        hideWord(words[i], 'V', crossword, i);
      }
    }
  }
}

void printCrossword(char crossword[15][10]) {
  printf("\nCROSSWORD\n");
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 11; j++) {
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

void printClues() {
  printf("\nHORIZONTAL\n");
  printf("1. %s\n", words[0][0]);
  printf("2. %s\n", word_second_horizontal[0]);
  printf("3. %s\n", word_third_horizontal[0]);

  printf("\nVERTICAL\n");
  printf("4. %s\n", word_first_vertical[0]);
  printf("5. %s\n", word_second_vertical[0]);
  printf("6. %s\n", word_third_vertical[0]);
}

int main() {
  char crossword[15][10];
  char solvedCrossword[15][10];

  updateCrossword(crossword, 1);
  updateCrossword(solvedCrossword, 0);

  printf("*** Welcome ***\n");
  printf("This is \"La casa de hojas\"\n");

  printCrossword(crossword);
  printClues();

  // strncpy(word_first_horizontal[0], "walk", 4);

  return 0;
}
