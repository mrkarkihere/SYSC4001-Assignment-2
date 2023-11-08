/* 
    Author: Arun Karki 

    Student ID: 101219923

    Date: Nov 8, 2023

*/

#include "shared_lib.h"

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
void message_send(struct msg_data* data){
    int msgqid = message_get();
    char temp_data[BUFFER_SIZE];

    printf("SENDING TO (CLIENT_PID %d): ", data->client_pid);
    fgets(temp_data, BUFFER_SIZE, stdin);
    strcpy(data->data, temp_data);

    if(msgsnd(msgqid, (void*) data, sizeof(*data), 0) == -1){
        perror("msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
}

// msg_rcv() -> # bytes received or -1
void message_receive(struct msg_data *data){
    int msgqid = message_get();

    // make changes to msg_key to properply function request-response model
    if(msgrcv(msgqid, (void*) data, sizeof(*data), data->msg_type, 0) == -1){
        perror("msgrcv failed\n");
        exit(EXIT_FAILURE);
    }
    printf("RECEIVED FROM (CLIENT_PID %d): %s\n", data->msg_type, data->data);
}

int main(){
    int running = 1;
    int msgqid;

    struct msg_data data;
    char buffer[BUFFER_SIZE];

    msgqid = message_get();

    while(running){
        
        // handle request first
        data.msg_type = 1; // server receives all type 1
        message_receive(&data); // data.msg_type now holds the client's pid

        // respond now
        data.msg_type = data.client_pid; // send to client that sent the request
        message_send(&data);

        // end if "end" typed
        if(strncmp(data.data, "end", 3) == 0 || strncmp(buffer, "end", 3) == 0){
            running = 0;
        }
    }

    // remove queue
    msgctl(msgqid, IPC_RMID, 0);
    return 0;
}