#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/stat.h>

int MAX_LICENCES = 10;
int PORT = 8080;
int MAX_LENGTH = 1024;
int backlog = 5;

int create_and_bind_socket(int port) {
    int server_fd;
    struct sockaddr_in server_addr;
    int opt = 1;
    socklen_t socket_len = sizeof(opt);
    memset(&server_addr,0,sizeof(server_addr));
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(1); }
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, socket_len) < 0) 
    { perror("sockpot failled");return -1; }
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    int bind_result = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_result < 0) { perror("failled to bind socket"); exit(EXIT_FAILURE); }
    int listener = listen(server_fd, backlog);
    if (listener < 0){perror("listening failed"); return -1;}
    return server_fd;
}

int main(int arc, int *argv[])
{
    int connections;
    while (1)
    {
    if (connections < MAX_LICENCES)
    {
        connections +=1;
        
    }   
    else
    {
        perror("all connection in use");
    }
    }
    return 0;
}