#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


int con(char *host, char *port, int flag) {
	int srv, yes = 1;
	struct addrinfo hints, *res, *r;
	int (*func)(int, const struct sockaddr *, socklen_t);

	/* Do we wanna use bind or connect? */
	if(flag)
		func = connect;
	else
		func = bind;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if(getaddrinfo(host, port, &hints, &res) != 0) {
		fprintf(stderr, "Error: cannot resolve hostname %s\n", host);
		return -1;
	}
	for(r = res; r; r = r->ai_next) {
		if((srv = socket(r->ai_family, r->ai_socktype, r->ai_protocol)) < 0)
			continue;

		setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		
		if(func(srv, res->ai_addr, res->ai_addrlen) == -1) {
			close(srv);
			fprintf(stderr, "Error binding/connecting!\n");
			continue;
		}
		break;
	}
	freeaddrinfo(res);
	if(!r) {
		fprintf(stderr, "Error: cannot connect to host %s\n", host);
		return -1;
	}
	return srv;
}
