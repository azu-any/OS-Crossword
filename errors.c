// Azuany Mila Ceron - 177068
// Final Project - Operating Systems
// Spring 2024
// Last modified: 2024/04/21
//

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void check_error(int fd) {
  if (fd < 0) {

    if (errno == EAGAIN) {
      printf("Error: archivo bloqueado");
    }

    else if (errno == EACCES) {
      printf("Error: problema de permisos para acceder al archivo.");
    } else if (errno == EBADF) {
      printf("Error: mal descriptor de archivos");
    } else {
      perror("Error:");
    }

    exit(-1);
  }
  return;
}
