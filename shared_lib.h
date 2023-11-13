/**
 * @file consumer.c
 * @author Arun Karki
 * @date 2023-11-12
 * 
 */
#ifndef SHARED_LIB_H
#define SHARED_LIB_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <math.h>
#include "semun.h"

#define BUFFER_SIZE 512
#define SHM_BUFFER_SIZE 1024

#define QUEUE_KEY (key_t) 6666 // msg queue
#define SERVER_MSG_TYPE 1
#define NUM_BUFFER 10 // making N buffers in memory
#define NUM_SEMAPHORE 3 // make 3 semaphores (0: Empty, 1: Full, 2: Mutex, 3: Destroy)

#define QUEUE_PERMISSION 0666 | IPC_CREAT
#define SHM_PERMISSION 0666 | IPC_CREAT
#define SEM_PERMISSION 0666 | IPC_CREAT

#define FILE_SIZE_REQ_MSG "file_size"
#define SH_MEM_REQ_MSG "sh_key"
#define SEM_REQ_MSG "sem_key"

#define TERMINATION_CODE "$$term$$"

// semaphore functions
static int set_semvalue(key_t* sem_id);
static void del_semvalue(key_t* sem_id);
static int semaphore_p(key_t* sem_id, int sem_num);
static int semaphore_v(key_t* sem_id, int sem_num);

struct msg_data{
    long int msg_type;
    pid_t client_pid;
    char message[BUFFER_SIZE]; // lets try to put the keys in here
};

struct shm_buffer{
    char message[SHM_BUFFER_SIZE];
    int sequence_number; // basically says  where im at reading/writing in total
    int count; // how much bytes im reading/writing in the buffer
};

#endif


  