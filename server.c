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

    sprintf(data->message, "RESPONSE;semS;1234");

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

    char request[BUFFER_SIZE]; // whats the request; sem or key, etc... ex 'semS'
    sscanf(data->message, "REQUEST;%s", request); // i guess it stores it here now? i dont really get it atm

    printf("SERVER RECEIVED:\n");
    printf("data->message: %s\n",data->message);
    printf("request: %s\n", request);
}

int main(){

    int msgqid;
    struct msg_data data;

    msgqid = message_get();

    // handle request first
    data.msg_type = 1; // server receives all type 1
    message_receive(&data); // data.msg_type now holds the client's pid

    // respond now
    data.msg_type = data.client_pid; // send to client that sent the request
    message_send(&data);

    sleep(1);
    // remove queue
    msgctl(msgqid, IPC_RMID, 0);

    return 0;
}