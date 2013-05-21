#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "util.h"

#define BACKLOG 10
#define NMAX 1024

struct commands {
	char *name;
	void (*func)(int, char *);
};

void reverse(int socket, char *data) {
	char tmp, *p;
	if(data == NULL || !(*data))
		return;
	p = data + strlen(data) - 1;
	while(p > data) {
		tmp = *data;
		*data = *p;
		*p = tmp;
		data++;
		p--;
	}
}
void whoami(int s, char *data) {
	snprintf(data, NMAX, "%d\n", s);
}
	


int main(int argc, char *argv[]) {
	int s, inc, numfds;
	struct sockaddr_storage sinc;
	struct commands cmd[] = {	{"reverse", reverse},
								{"whoami", whoami}
							};
	fd_set md, rd;
	
	if(argc < 3) {
		fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
		return 1;
	}
	s = con(argv[1], argv[2], 0);

	if(listen(s, BACKLOG) == -1) {
		close(s);
		fprintf(stderr, "Error listening!\n");
		return 1;
	}

	numfds = s;

	FD_ZERO(&md);
	FD_ZERO(&rd);
	FD_SET(s, &md);

	while(1) {
		int msglen, i;
		socklen_t addr_len;
		char *data = malloc(sizeof(char) * NMAX);
		rd = md;

		if(select(numfds+1, &rd, 0, 0, 0) == -1) {
			fprintf(stderr, "Error in select()\n");
			return 1;
		}

		for(i = 0; i <= numfds; i++) {
			if(FD_ISSET(i, &rd)) {
				if(i == s) { /* New connection */
					addr_len = sizeof sinc;
					if((inc = accept(s, (struct sockaddr *)&sinc, &addr_len)) == -1) {
						fprintf(stderr, "Error in accept()\n");
					}
					else {
						FD_SET(inc, &md);
						if(inc > numfds)
							numfds = inc;
						printf("Connected by someone!\n");
					}
				}
				else { /* Old connection, handle data */
					if((msglen = recv(i, data, NMAX-1, 0)) <= 0) {
						if(msglen == 0)
							fprintf(stderr, "Socket %d disconnected.\n", i);
						else
							fprintf(stderr, "Error in recv()\n");
						close(i);
						FD_CLR(i, &md);
					}
					else {
						char *index;
						int j;
						data[msglen] = '\0';
						printf("Got data: %s\nProcessing and sending...\n", data);

						if((index = strchr(data, ';')) != NULL) {
							for(j = 0; j < sizeof(cmd)/sizeof(cmd[0]); j++) {
								if(!strncmp(data, cmd[j].name, index-data)) {
									data = index + 1;
									cmd[j].func(i, data);
									break;
								}
							}
							if(!strncmp(data, "broadcast", index-data)) {
								printf("Sending broadcast...\n");
								data = index + 1;
								for(j = 0; j <= numfds; j++) {
									if(FD_ISSET(j, &md)) {
										if(j != s && j != i) {
											send(j, data, strlen(data), 0);
										}
									}
								}
							}
						}
						send(i, data, strlen(data), 0);
					}
				}
			}
		}
	}

	close(s);

	return 0;
}
