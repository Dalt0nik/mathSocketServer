#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

#define PORT "20006"
#define BUF_SIZE 1024 

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void strtoupper(char *str) {
    for (; *str; str++)
        *str = toupper(*str);
}

int generate_problem() {
    srand(time(NULL));
    int num1 = rand() % 10 + 1;
    int num2 = rand() % 10 + 1;
    return num1 * num2;
}

int main() {
    int server_socket, client_socket;
    struct addrinfo hints, *server_info;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size;
    char client_ip[INET6_ADDRSTRLEN];
    int option_yes = 1;
    int option_no = 0;
    char buffer[BUF_SIZE];
    int bytes_received;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; 
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, PORT, &hints, &server_info) != 0) {
        perror("getaddrinfo error");
        exit(1);
    }
    
    server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (server_socket == -1) {
      perror("server: socket");
      exit(1);
    }

    if (setsockopt(server_socket, IPPROTO_IPV6, IPV6_V6ONLY, &option_no, sizeof(option_no)) == -1) {
      perror("setsockopt IPV6_V6ONLY");
      exit(1);
    }

    if (bind(server_socket, server_info->ai_addr, server_info->ai_addrlen) == -1) {
      perror("server: bind");
      exit(1);
    }

    if (listen(server_socket, 10) == -1) { 
      perror("listen");
      exit(1);
    }

    printf("server: waiting for connections!\n");

    while(1) { // main accept() loop
      client_addr_size = sizeof client_addr;
      client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
      if (client_socket == -1) {
        perror("accept");
        continue;
      }

      inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), client_ip, sizeof client_ip);
      printf("server: got connection from %s\n", client_ip);


      // Send numbers from 1 to 10 every 10 seconds
      // for (int i = 1; i <= 10; ++i) {
      //   snprintf(buffer, sizeof(buffer), "%d", i);
      //   if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
      //     perror("send");
      //     break;
      //   }
      //   printf("Sent: %s\n", buffer);
      //   sleep(10); // Wait for 10 seconds
      // }

      while((bytes_received = recv(client_socket, buffer, BUF_SIZE-1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("Received: %s\n", buffer);
        strtoupper(buffer);
        if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
          perror("send");
        }
      }

      if (bytes_received == -1) {
        perror("recv");
        close(client_socket);
        continue;
      }

      close(client_socket); 
    }

    freeaddrinfo(server_info); 
}
