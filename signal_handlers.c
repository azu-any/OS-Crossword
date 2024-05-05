#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void sig_handler_sigint(int signum) {
	printf("See you later\n");
	exit(0);
}
