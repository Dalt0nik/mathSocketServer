#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#define PORT "20006"
#define BUF_SIZE 1024

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void strtoupper(char *str)
{
    for (; *str; str++)
        *str = toupper(*str);
}

int main()
{
    int server_socket, client_socket;
    struct addrinfo hints, *server_info;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size;
    char client_ip[INET6_ADDRSTRLEN];
    int option_yes = 1;
    int option_no = 0;
    char buffer[BUF_SIZE];
    int bytes_received;
    int client_answer;
    int correct_answer;
    int score = 0;
    fd_set read_fds;
    struct timeval timeout;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, PORT, &hints, &server_info) != 0)
    {
        perror("getaddrinfo error");
        exit(1);
    }

    server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (server_socket == -1)
    {
        perror("server: socket");
        exit(1);
    }

    if (setsockopt(server_socket, IPPROTO_IPV6, IPV6_V6ONLY, &option_no, sizeof(option_no)) == -1)
    {
        perror("setsockopt IPV6_V6ONLY");
        exit(1);
    }

    if (bind(server_socket, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        perror("server: bind");
        exit(1);
    }

    if (listen(server_socket, 10) == -1)
    {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections!\n");

    while (1)
    { // main accept() loop
        client_addr_size = sizeof client_addr;
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_socket == -1)
        {
            perror("accept");
            continue;
        }

        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), client_ip, sizeof client_ip);
        printf("server: got connection from %s\n", client_ip);

        // Send 10 multiplication problems with time limit for each
        for (int i = 0; i < 10; ++i)
        {
            int a = rand() % 10 + 1; // Random numbers between 1 and 10
            int b = rand() % 10 + 1;
            correct_answer = a * b;

            snprintf(buffer, sizeof(buffer), "Question %d: %d * %d = ?", i + 1, a, b);
            if (send(client_socket, buffer, strlen(buffer), 0) == -1)
            {
                perror("send");
                break;
            }

            // Set socket to non-blocking mode
            int flags = fcntl(client_socket, F_GETFL, 0);
            fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);

            
            FD_ZERO(&read_fds); //empty file descriptor pool
            FD_SET(client_socket, &read_fds); //add my client_socket fd
            timeout.tv_sec = 10; // 10 seconds timeout
            timeout.tv_usec = 0;
            if (select(client_socket + 1, &read_fds, NULL, NULL, &timeout) > 0) //max fd, fd_set for read, write, err, timeval
            {
                //happens when there're things to do on our fd set

                //now it's a default reading+sending process
                bytes_received = recv(client_socket, buffer, BUF_SIZE - 1, 0);
                if (bytes_received > 0)
                {
                    buffer[bytes_received] = '\0';

                    
                    client_answer = atoi(buffer);

                    // Check client's answer
                    if(client_answer == 0) //not a number
                    {
                      if (send(client_socket, "Answer is not valid!\n", 21, 0) == -1)
                      {
                          perror("send");
                          break;
                      }
                    }
                    else if (client_answer == correct_answer)
                    {
                      score++;
                      if (send(client_socket, "Correct!\n", 9, 0) == -1)
                      {
                          perror("send");
                          break;
                      }
                    }
                    else
                    {
                      if (send(client_socket, "Incorrect!\n", 11, 0) == -1)
                      {
                          perror("send");
                          break;
                      }
                    }
                }
            }
            else
            {
                // Timeout reached, send message to client
                if (send(client_socket, "\nTimeout!\n", 10, 0) == -1)
                {
                    perror("send");
                    break;
                }
            }

            // Set socket back to blocking mode
            fcntl(client_socket, F_SETFL, flags);
        }

        snprintf(buffer, sizeof(buffer), "Score %d/%d\n", score, 10);
        if (send(client_socket, buffer, strlen(buffer), 0) == -1)
        {
            perror("send");
            break;
        }

        close(client_socket);
    }

    freeaddrinfo(server_info);
}
