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

void* threadHandler(void *arg)
{
    pthread_detach(pthread_self);
    int cliend_fd = *(int*)arg;
    free(arg);

    char buffer[1024];
    int byte_read= recv(cliend_fd,buffer, sizeof(buffer) - 1,0);
    if(byte_read > 0)
    {
        buffer[byte_read] = "\0";
        printf("recived: %s\n", buffer);
        send(cliend_fd, "ACK\n", 4, 0);
    }

    clsoe(cliend_fd);
    retunr NULL;
}

int create_and_bind_socket(int port) {
    int server_fd;
    struct sockaddr_in server_addr;
    int opt = 1;
    int listener = listen(server_fd, backlog);
    if (listen < 0){perror("listening failed"); return -1;}
    memset(&server_addr,0,sizeof(server_addr));
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(1); }
    server_addr.sin_port = htons(port);
    int bind_result = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_result < 0) { perror("failled to bind socket"); exit(EXIT_FAILURE); }
    socklen_t socket_len = sizeof(opt);
    if(setsockopt(server_fd, IPPROTO_TCP, SO_REUSEADDR, &opt, opt) < 0) 
    { perror("sockpot failled");return -1; }
    return server_fd;
}

void sigint_handler(int signo)
{

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