#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/time.h>

int PORT = 8080;
int MAX_LENGTH = 1024;
int backlog = 5;
volatile sig_atomic_t shutdown_flag = 0;

typedef struct {
    // mutex, active_count, max_clients, shutdown_flag
    pthread_mutex_t lock;
    int active_count;
    int max_clients;
    volatile sig_atomic_t shutdown_flag;
} shared_state_t;

typedef struct {
    // client_fd, pointer to shared_state_t
    int client_fd;
    shared_state_t *shared;
} client_arg_t;

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
    //binds to the port
    int bind_result = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_result < 0) { perror("failled to bind socket"); exit(EXIT_FAILURE); }
    //listens for a data
    int listener = listen(server_fd, backlog);
    if (listener < 0){perror("listening failed"); return -1;}
    return server_fd;
}

void sigint_handler(int signo) {
    shutdown_flag = 1;
}

void *client_handler(void *arg)
{
    client_arg_t *my_arg = (client_arg_t *)arg;
    int fd = my_arg->client_fd;
    char buff[MAX_LENGTH];
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    shared_state_t *shared = my_arg->shared;
    while(1)
    {
        ssize_t bytes = recv(fd, buff, sizeof(buff), 0);
        if (bytes > 0) {
        // echo back, maybe printf/log
        send(fd, buff, bytes, 0);
        } else if (bytes == 0) {
        break;  // peer disconnected
        } 
        else 
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN) 
            {
                if (shutdown_flag) break;
                    continue;
            } 
            else   
            {
                perror("recv failed");
                break;
            }
        }
    }
    close(fd);
    pthread_mutex_lock(&shared->lock);
    shared->active_count--;
    pthread_mutex_unlock(&shared->lock);
    free(my_arg);
    return NULL;
}


void wait_for_active_connections(shared_state_t *state, int timeout_seconds)
{
    int count = 0;
    int max_iterations = (timeout_seconds * 1000) / 100;
    while(1){
        pthread_mutex_lock(&state->lock);
        int some_value = state->active_count;
        pthread_mutex_unlock(&state->lock);
        if(some_value == 0){return;}//no connections open
        if(count == max_iterations){return;}//maximum amount of itiration

        usleep(100000);  // sleep 100ms
        count++;
    }
}

int main(int arc, char *argv[])
{
    pthread_t thread;
    shared_state_t state;
    state.active_count = 0;
    state.max_clients = 10;
    state.shutdown_flag = 0;
    struct sockaddr_in client_addr;
    int client_fd;
    int mutex_result = pthread_mutex_init(&state.lock, NULL);
    if (mutex_result != 0) { perror("mutex init fa  iled"); return -1; }
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;  // deliberately no SA_RESTART

sigaction(SIGINT, &sa, NULL);
sigaction(SIGTERM, &sa, NULL);
    int listen_fd = create_and_bind_socket(PORT);
    if (listen_fd == -1){perror("failled to bind/ crete socket"); return -1;}
    while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);

    if (client_fd == -1) {
        if (errno == EINTR) {
            if (shutdown_flag) {
                break;
            }
            continue;
        } else {
            perror("accept failed");
            continue;
        }
    } else {
        pthread_mutex_lock(&state.lock);
        if (state.active_count < state.max_clients) {
            state.active_count++;
            pthread_mutex_unlock(&state.lock);

            client_arg_t *new_client = malloc(sizeof(client_arg_t));
            if (new_client == NULL) {
                perror("malloc failed");
                close(client_fd);
                continue;
            }
            new_client->client_fd = client_fd;
            new_client->shared = &state;

            pthread_create(&thread, NULL, client_handler, new_client);
            pthread_detach(thread);
        } else {
            pthread_mutex_unlock(&state.lock);
            printf("no more connections left\n");
            close(client_fd);
        }
    }
}

close(listen_fd);
wait_for_active_connections(&state, 5);
return 0;
}