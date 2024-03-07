/*
** client.c -- a stream socket client demo
*/

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

#define PORT "20006"

#define MAX_MESSAGE_SIZE 1024

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int socket_fd, num_bytes;
    char received_message[MAX_MESSAGE_SIZE];
    struct addrinfo hints, *server_info, *p;
    int rv;
    char server_ip[INET6_ADDRSTRLEN];
    char user_input[MAX_MESSAGE_SIZE];

    if (argc != 2)
    {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &server_info)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for (p = server_info; p != NULL; p = p->ai_next)
    {
        if ((socket_fd = socket(p->ai_family, p->ai_socktype,
                                p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }

        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("client: connect");
            close(socket_fd);
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              server_ip, sizeof server_ip);
    printf("client: connecting to %s\n", server_ip);

    freeaddrinfo(server_info); 


	printf("Enter a message to send to the server: ");
	fgets(user_input, MAX_MESSAGE_SIZE, stdin);

	
	size_t len = strlen(user_input);
	if (len > 0 && user_input[len - 1] == '\n')
		user_input[len - 1] = '\0';

    // send to server
    if (send(socket_fd, user_input, strlen(user_input), 0) == -1)
    {
        perror("send");
        close(socket_fd);
        exit(1);
    }

    // Receive response from server
    if ((num_bytes = recv(socket_fd, received_message, MAX_MESSAGE_SIZE - 1, 0)) == -1)
    {
        perror("recv");
        exit(1);
    }

    received_message[num_bytes] = '\0';

    printf("client: received \"%s\"\n", received_message);

    close(socket_fd);

    return 0;
}
