#include "shared_lib.h"

// msg_get() -> returns msgqid
int message_get(){
    int msgqid = msgget(KEY, 0666 | IPC_CREAT);

    if(msgqid == -1){
        perror("msgget failure\n");
        exit(EXIT_FAILURE);
    }

    printf("server online and accepting requests...\n");
    return msgqid;
}

// msg_snd() -> -1 or 0
void message_send(struct msg_data* data){
    int msgqid = message_get();
    char temp_data[BUFFER_SIZE];

    printf("SENDING TO (CLIENT_PID %d): ", data->pid);
    fgets(temp_data, BUFFER_SIZE, stdin);
    strcpy(data->data, temp_data);

    if(msgsnd(msgqid, (void*) data, sizeof(*data), 0) == -1){
        perror("msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
}

// msg_rcv() -> # bytes received or -1
void message_receive(struct msg_data *data, int* msg_key){
    int msgqid = message_get();

    // make changes to msg_key to properply function request-response model
    if(msgrcv(msgqid, (void*) data, sizeof(*data), *msg_key, 0) == -1){
        perror("msgrcv failed\n");
        exit(EXIT_FAILURE);
    }
    printf("RECEIVED FROM (CLIENT_PID %d): %s\n", data->pid, data->data);
}

int main(){
    int running = 1;
    int msg_key = 0; // this is complicated a little
    int msgqid;

    struct msg_data data;
    char buffer[BUFFER_SIZE];

    msgqid = message_get();

    while(running){
        // handle request first
        message_receive(&data, &msg_key);

        // respond now
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
