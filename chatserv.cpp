#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 10     

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

//sets up connection to client on port argv[1]
int setup(char* portno){
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, portno, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections...\n");

	sin_size = sizeof their_addr;
	//printf("server: waiting for client\n");
	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	    
    close(sockfd);

	if (new_fd == -1) {
		perror("accept");
		exit(1);
	}
	inet_ntop(their_addr.ss_family,
		get_in_addr((struct sockaddr *)&their_addr),
		s, sizeof s);
		
	return new_fd;
}

void chat(int socket_fd, char* clientHandle){
		char buf[MAXDATASIZE];
		int numbytes;
		int quit;
		while(1){
			//recv
			memset(buf, 0, MAXDATASIZE);
			if ((numbytes = recv(socket_fd, buf, MAXDATASIZE-1, 0)) == -1) {
				perror("recv");
				exit(1);
			}
			if (strncmp(buf, "Connection closed by Client", 27) == 0){
				printf("%s\n", buf);
				close(socket_fd);
				exit(0);
			}
    		buf[numbytes-1] = '\0';
    		printf("%s> %s\n", clientHandle, buf);
    		
			//send
			memset(buf, 0, MAXDATASIZE);
			printf("Server> ");
			fgets(buf, MAXDATASIZE-1, stdin);
			quit = strncmp(buf, "\\quit", 4);
			if (quit == 0){
				if(send(socket_fd, "Connection closed by Server\n", 28, 0) == -1){
					perror("send");
				}
				close(socket_fd);
				exit(0);
			}
			else{
				if(send(socket_fd, buf, strlen(buf), 0) == -1){
					perror("send");
				}
			}
  
		}
		return;
}

int main(int argc, char *argv[])
{
    int new_fd; 
    int numbytes;
    char clientHandle[MAXHANDLESIZE];
            
    if (argc != 2) {
        fprintf(stderr,"usage: server <portnumber>\n");
        exit(1);
    }
    
    //connect to Client
    new_fd = setup(argv[1]);

	
	if ((numbytes = recv(new_fd, clientHandle, MAXDATASIZE-1, 0)) == -1) {
				perror("recv");
				exit(1);
	}	
	
	clientHandle[numbytes] = '\0';
	printf("server: got connection from %s\n", clientHandle);
	printf("Type '\\quit' to quit at any time\n\n");
	chat(new_fd, clientHandle); 

    return 0;
}