#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>


#define PAGE_SIZE 4096
#define ServerFifo "server.fifo"


typedef struct {
	pid_t pid;
	char name[256];
}message;

int main(int argc, char *argv[]) {
	int sd = -1, fh = -1, rd = -1, wr = -1;	
	char FIFO[64];
	
	void *buf = NULL;
	
	if ((buf = malloc(PAGE_SIZE)) == NULL) {
		fprintf(stderr, "OOM\n");
		return -1;
	}
    
	if (argc != 2) {
		perror("Please, make sure, your arguments are correct!\n");
		return errno;
	}

	sprintf(FIFO, "%d", getpid());

	message msg;
	msg.pid = getpid();
	strcpy(msg.name, argv[1]);

	umask(0);
	if (mkfifo(FIFO, 0666) == -1 && errno != EEXIST) {
		perror("error making the fifo");
		return errno;
	}


	if ((sd = open(ServerFifo, O_WRONLY)) < 0) {
		perror("error opening the server fifo");
		return errno;
	}

	if ((write(sd, &msg, sizeof(message))) < 0) {
		perror("error sending message to server");
		return errno;
	}

	if ((fh = open(FIFO, O_RDONLY)) < 0) {
		perror("open fifo");
		return errno;
	}

	while (1) {
		if ((rd = read(fh, buf, PAGE_SIZE)) < 0) {
			perror("error reading information from server");
			return errno;
		}
		
		if (rd == 0) 
			break;
		
		if ((wr = write(1, buf, rd)) < 0) {
			perror("parent write");
			return errno;
		}
	}
	
	if((close(fh)) < 0) {
		perror("error closing the client fifo's file");
		return errno;
	}
	
	
	if((close(sd)) < 0) {
		perror("error closing the server fifo's file");
		return errno;
	}
	
	return 0;
}
