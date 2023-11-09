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
void message_send(struct msg_data* data, char* request){
    int msgqid = message_get();
    char temp_data[BUFFER_SIZE];

    sprintf(data->message, request); // this is the request message being sent to the server

    data->client_pid = getpid(); // server will use this to write back

    if(msgsnd(msgqid, (void*) data, sizeof(*data), 0) == -1){
        perror("msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
}

// msg_rcv() -> # bytes received or -1
key_t message_receive(struct msg_data *data){
    int msgqid = message_get();

    if(msgrcv(msgqid, (void*) data, sizeof(*data), data->msg_type, 0) == -1){
        perror("msgrcv failed\n");
        exit(EXIT_FAILURE);
    }

    int key; // response sent from server
    sscanf(data->message, "RESPONSE;%d", &key);

    printf("key received: %d\n", key);
    return (key_t) key;
}

int main(){

    int msgqid;
    struct msg_data data;
    key_t shm_key, sem_key;
    char request[BUFFER_SIZE]; // what type of request am i making

    msgqid = message_get();

    // request for shared memory key from server
    data.msg_type = SERVER_MSG_TYPE; // send a type 1 for server to read
    sprintf(request, "REQUEST;sh_key");
    message_send(&data, &request);

    // shared memory key received from server
    data.msg_type = getpid(); // receive to client that sent the request
    shm_key = message_receive(&data);
    
    return 0;
}