#ifndef SHARED_LIB_H
#define SHARED_LIB_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>

#define BUFFER_SIZE 512
#define QUEUE_KEY (key_t) 6666 // msg queue
#define QUEUE_PERMISSION 0666 | IPC_CREAT
#define SERVER_MSG_TYPE 1;


struct msg_data{
    long int msg_type;
    pid_t client_pid;
    char data[BUFFER_SIZE];
};

#endif
  