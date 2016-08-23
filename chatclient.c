#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAXDATASIZE 500  
#define MAXHANDLESIZE 10 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int setup(char* hostname, char* portno){
	int sockfd;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN]; 
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname, portno, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        exit(1);
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure
	
	return sockfd;
}

void chat(int socket_fd, char* handle){
	char buf[MAXDATASIZE];
	int numbytes;
	int quit;
	while(1){
	
		//send
		memset(buf, 0, MAXDATASIZE);
		printf("%s> ", handle);
		fgets(buf, MAXDATASIZE-1, stdin);
		quit = strncmp(buf, "\\quit", 4);
		if (quit == 0){
			if(send(socket_fd, "Connection closed by Client\n", 28, 0) == -1){
				perror("send");
				exit(1);
			}
			close(socket_fd);
			exit(0);
		}
		else{
			if(send(socket_fd, buf, strlen(buf), 0) == -1){
				perror("send");
				exit(1);
			}
		}
		
		//recv
		if ((numbytes = recv(socket_fd, buf, MAXDATASIZE-1, 0)) == -1) {
			perror("recv");
			exit(1);
		}
		if (strncmp(buf, "Connection closed by Server", 27) == 0){
			printf("%s\n", buf);
			close(socket_fd);
			exit(0);
		}
		buf[numbytes-1] = '\0';
		printf("Server> %s\n",buf);
	}
	return;
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    char handle[MAXHANDLESIZE];
	size_t ln;
    
    if (argc != 3) {
        fprintf(stderr,"usage: client hostname portnumber\n");
        exit(1);
    }

	//connect to Server
	sockfd = setup(argv[1], argv[2]);

	//get handle and send to Server
	printf("Enter a handle up to %d characters: ", MAXHANDLESIZE);
	fgets(handle, MAXHANDLESIZE, stdin);
	ln = strlen(handle) - 1;
	if (handle[ln] == '\n')
    handle[ln] = '\0';
    if(send(sockfd, handle, strlen(handle), 0) == -1){
		perror("send");
		exit(1);
	}
	
	printf("Type '\\quit' to quit at any time\n\n");
	chat(sockfd, handle);      

    close(sockfd);

    return 0;
}