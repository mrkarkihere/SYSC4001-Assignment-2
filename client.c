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

    sprintf(data->message, "REQUEST;semS"); // this is the request message being sent to the server

    data->client_pid = getpid(); // server will use this to write back

    if(msgsnd(msgqid, (void*) data, sizeof(*data), 0) == -1){
        perror("msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
}

// msg_rcv() -> # bytes received or -1
void message_receive(struct msg_data *data){
    int msgqid = message_get();

    if(msgrcv(msgqid, (void*) data, sizeof(*data), data->msg_type, 0) == -1){
        perror("msgrcv failed\n");
        exit(EXIT_FAILURE);
    }

    char response[BUFFER_SIZE];
    int key;

    sscanf(data->message, "RESPONSE;%[^;];%d", response, &key);
    printf("CLIENT RECEIVED:\n");
    printf("data->message: %s\n",data->message);
    printf("response: %s\n", response);
    printf("key: %d\n", key);
}

int main(){

    int msgqid;
    struct msg_data data;
    
    msgqid = message_get();

    // respond now
    data.msg_type = SERVER_MSG_TYPE; // send a type 1 for server to read
    message_send(&data);

    // handle request first
    data.msg_type = getpid(); // receive to client that sent the request
    message_receive(&data);

    return 0;
}