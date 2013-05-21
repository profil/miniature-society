#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "util.h"

#define NMAX 1024

int main(int argc, char *argv[]) {
	int s;
	char msg[NMAX], data[NMAX];
	fd_set rd;

	if(argc < 3) {
		fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
		return 1;
	}


	s = con(argv[1], argv[2], 1);

	while(1) {
		int i;
		FD_ZERO(&rd);
		FD_SET(0, &rd); /* stdin */
		FD_SET(s, &rd);
		
		if((i = select(s + 1, &rd, 0, 0, 0)) == -1) {
			fprintf(stderr, "Error in select()\n");
			return 1;
		}
		if(FD_ISSET(s, &rd)) {
			int datalen;
			datalen = recv(s, data, NMAX, 0);
			data[datalen] = '\0';
			printf("got> %s\n", data);
		}
		if(FD_ISSET(0, &rd)) {
			if(fgets(msg, NMAX, stdin) == NULL)
				break;
			send(s, msg, strlen(msg) - 1, 0);
		}
	}
	close(s);

	return 0;
}
