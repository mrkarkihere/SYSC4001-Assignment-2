#include "shared_lib.h"
#include <time.h>

// global variables
key_t shm_key = -1;
key_t sem_key = -1;

// generate a random key
int generate_key(){
    printf("generating a key...\n");
    //srand(time(NULL));
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
    char temp_data[BUFFER_SIZE];

    // check to see if requested shared memory key
    if(strncmp(request, SH_MEM_REQ_MSG, strlen(request)) == 0){
        // if key is -1 then generate a key
        if(shm_key == -1) shm_key = generate_key();
        sprintf(data->message, "RESPONSE;%d", shm_key);
        printf("shm_key: %d\n", shm_key);

    // check to see if requested semaphore key
    }else if(strncmp(request, SEM_REQ_MSG, strlen(request)) == 0){
        // if key is -1 then generate a key
        if(sem_key == -1) sem_key = generate_key();
        sprintf(data->message, "RESPONSE;%d", sem_key);
        printf("sem_key: %d\n", sem_key);
    }

    if(msgsnd(msgqid, (void*) data, sizeof(*data), 0) == -1){
        perror("msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
}

// msg_rcv() -> # bytes received or -1
void message_receive(struct msg_data *data, char* request){
    int msgqid = message_get();

    // make changes to msg_key to properply function request-response model
    if(msgrcv(msgqid, (void*) data, sizeof(*data), data->msg_type, 0) == -1){
        perror("msgrcv failed\n");
        exit(EXIT_FAILURE);
    }

    sscanf(data->message, "REQUEST;%s", request); // i guess it stores it here now? i dont really get it atm
    printf("request: %s\n", request);
}

int main(){

    srand(time(NULL)); // seed

    int msgqid;
    struct msg_data data;
    char request[BUFFER_SIZE]; // what type of request is being made

    msgqid = message_get();

    while(1){
    // handle request first
    data.msg_type = 1; // server receives all type 1
    message_receive(&data, &request); // data.msg_type now holds the client's pid

    // respond now
    data.msg_type = data.client_pid; // send to client that sent the request
    message_send(&data, &request);
    }

    // remove queue
    sleep(1);
    msgctl(msgqid, IPC_RMID, 0);

    return 0;
}