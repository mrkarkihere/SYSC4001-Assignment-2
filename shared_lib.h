#ifndef SHARED_LIB_H
#define SHARED_LIB_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>

#define BUFFER_SIZE 512
#define KEY (key_t) 1234 // msg key

struct msg_data{
    long int pid;
    char data[BUFFER_SIZE];
};

#endif
  