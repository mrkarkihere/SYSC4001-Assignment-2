#include "shared_lib.h"

// msg_get() -> returns msgqid
int message_get(){
    int msgqid = msgget(KEY, 0666 | IPC_CREAT);

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

    printf("SENDING TO (SERVER): ");
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
    /* NOTE:
        msg_key(4th param): is the type of message aka who can read etc
    */
    if(msgrcv(msgqid, (void*) data, sizeof(*data), 1, 0) == -1){
        perror("msgrcv failed\n");
        exit(EXIT_FAILURE);
    }
    printf("RECEIVED FROM (SERVER): %s", data->data);
}

int main(){
    int running = 1;
    int msgqid;

    struct msg_data data;
    char buffer[BUFFER_SIZE];

    msgqid = message_get();
    data.pid = getpid();

    while(running){
        // respond now
        message_send(&data);

        // handle request first
        message_receive(&data);

        // end if "end" typed
        if(strncmp(data.data, "end", 3) == 0 || strncmp(buffer, "end", 3) == 0){
            running = 0;
        }
    }
    return 0;
}
