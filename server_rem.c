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
	int wr = -1, rd = -1, fh = - 1, rd_msg = -1;
	
	char FIFO[64];
	
	message msg;
       
	signal(SIGINT, SIG_DFL);
             
	void *buf = NULL; 
	
	if ((buf = malloc(PAGE_SIZE)) == NULL) {
		fprintf(stderr, "OOM\n");
		return -1;
	}    
	
	umask(0);                           
	if (mkfifo(ServerFifo, 0666) == -1 && errno != EEXIST) {
		perror("mkfifo");
		return errno;
	}

   
	if ((rd_msg = open(ServerFifo, O_RDONLY)) < 0) {
		perror("open fifo");
		return errno;
	}

	while(1) {            
		if (read(rd_msg, &msg, sizeof(message)) != sizeof(message)) {
			fprintf(stderr, "Error reading message\n");
			continue;                  
		}
		
		sprintf(FIFO, "%d", msg.pid);
		
		if ((fh = open(msg.name, O_RDONLY)) < 0) {
			perror("stderr, Error opening file\n");
			continue;
       	}
		
		if ((wr = open(FIFO, O_WRONLY)) < 0) {
			perror("stderr, opening client fifo\n");
			continue;
		}


		while (1) {
            		if((rd = read(fh, buf, PAGE_SIZE)) < 0) {
                		perror("reading the file");
                		break;
            		}

            		if(!rd) 
            			break;

            		if (write(wr, buf, rd) < 0) {
                		perror("sending information to client");
                		return errno;
            		}    
		}
        	
		if((close(fh)) < 0) {
			perror("closing the file received from client\n");
		}	
		
  		if((close(wr)) < 0) {
			perror("closing the client fifo's file");
		}
  		
	}
	
	if((close(rd_msg)) < 0) {
		perror("closing the server fifo's file");
	}
	
	if (unlink(ServerFifo) < 0) {
		perror("deleting fifo");
	}
	
	return 0;
}
