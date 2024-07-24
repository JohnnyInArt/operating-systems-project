#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

// Constants
#define BUFFER_SIZE 1024
#define SERVERPORT 50010
#define SERVERADDRESS "127.0.0.1"

// Function Prototypes
void sendMessage(int clientFd, const char *message);
int getInput(char *buffer, size_t size);
void readMenu(int clientFd);

#endif //TCPCLIENT_H
