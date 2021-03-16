#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BACKLOG 5 // length of listening queue
#define BUFFER_SIZE 16384

typedef struct {
    in_addr_t ip;
    int port;
    char file_path[256];
} arguments;

typedef struct {
    char path[256];
    uint64_t size;
} fileInfo;

/*declaration of funtions*/
void error(const char*);
void tcp_send(arguments);
void tcp_recv(arguments);
void udp_send(arguments);
void udp_recv(arguments);

#endif