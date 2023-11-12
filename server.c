#include "shared_lib.h"
#include <time.h>
#include <sys/stat.h>

// global variables
key_t shm_key = -1;
key_t sem_key = -1;

// generate a random key
int generate_key(){
    return rand() % (9999 - 1000 + 1) + 1000;
}

// msg_get() -> returns msgqid
int message_get(){
    int msgqid = msgget(QUEUE_KEY, QUEUE_PERMISSION);

    if(msgqid == -1){
        perror("msgget failure\n");
        exit(EXIT_FAILURE);
    }
    return msgqid;
}

// msg_snd() -> -1 or 0
void message_send(struct msg_data* data, char* request){

    int msgqid = message_get();
    char file_name[BUFFER_SIZE];
    sscanf(data->message, "REQUEST;%[^;];%s", request, file_name); // get file name if it exists

    // check to see if requested shared memory key
    if(strncmp(request, SH_MEM_REQ_MSG, strlen(request)) == 0){
        // if key is -1 then generate a key
        if(shm_key == -1) shm_key = generate_key();
        sprintf(data->message, "RESPONSE;%d", shm_key);
        printf("Sending %s = %d to (PID_%d)\n", SH_MEM_REQ_MSG, shm_key, data->client_pid);

    // check to see if requested semaphore key
    }else if(strncmp(request, SEM_REQ_MSG, strlen(request)) == 0){
        // if key is -1 then generate a key
        if(sem_key == -1) sem_key = generate_key();
        sprintf(data->message, "RESPONSE;%d", sem_key);
        printf("Sending %s = %d to (PID_%d)\n", SEM_REQ_MSG, sem_key, data->client_pid);
        
    // check to see if requested file size
    }else if(strncmp(request, FILE_SIZE_REQ_MSG, strlen(request)) == 0){
        struct stat file_info;
        stat(file_name, &file_info); // read info using file_name
        sprintf(data->message, "RESPONSE;%d", (int) file_info.st_size);
        printf("Sending %s = %d bytes to (PID_%d)\n", FILE_SIZE_REQ_MSG, (int) file_info.st_size, data->client_pid);
    }

    if(msgsnd(msgqid, (void*) data, sizeof(*data), 0) == -1){
        perror("msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
}

// msg_rcv() -> # bytes received or -1
int message_receive(struct msg_data *data, char* request){
    int msgqid = message_get();

    // make changes to msg_key to properply function request-response model
    if(msgrcv(msgqid, (void*) data, sizeof(*data), data->msg_type, 0) == -1){
        perror("msgrcv failed\n");
        exit(EXIT_FAILURE);
    }
    
    sscanf(data->message, "REQUEST;%s", request); // i guess it stores it here now? i dont really get it atm
    printf("Request Received (PID_%d): %s\n", data->client_pid, request);
    
    // check if termination
    if(strncmp(request, TERMINATION_CODE, strlen(request)) == 0)
        return 0;

    return 1;
}

int main(){

    printf("\n---- SERVER ----\n");

    srand(time(NULL)); // seed

    int running = 1;
    int msgqid;
    struct msg_data data;
    char request[BUFFER_SIZE]; // what type of request is being made

    msgqid = message_get();

    while(running){
        // handle request first
        data.msg_type = 1; // server receives all type 1
        running = message_receive(&data, &request); // data.msg_type now holds the client's pid

        // respond now
        data.msg_type = data.client_pid; // send to client that sent the request
        message_send(&data, &request);
    }

    // remove queue
    sleep(1);
    msgctl(msgqid, IPC_RMID, 0);
    printf("\n%s\n", TERMINATION_CODE);
    return 0;
}