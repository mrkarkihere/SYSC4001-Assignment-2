#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/msg.h>

#define MAX_TEXT 512

#define SERVER_MQUEUE 1234
#define CLIENT_MQUEUE 4321

struct my_msg_st
{
    long int msg_key;
    char some_text[BUFSIZ];
};

// msgget() call with all the stuff so no repeats
int message_get(int QUEUE_ID){

    int msgid = msgget((key_t)QUEUE_ID, 0666 | IPC_CREAT);
    
    if (msgid == -1){
        fprintf(stderr, "msgget failed with error: % d\n", errno);
        exit(EXIT_FAILURE);
    }

    return msgid;
}

// receive msg
void message_receive(struct my_msg_st* some_data, int* msg_to_receive)
{
    int msgid = message_get(SERVER_MQUEUE);
    if (msgrcv(msgid, (void *)some_data, BUFSIZ, *msg_to_receive, 0) == -1)
    {
        fprintf(stderr, "msgrcv failed with error: % d\n", errno);
        exit(EXIT_FAILURE);
    }
    printf("(CLIENT_PID %d) wrote: %s", some_data->msg_key, some_data->some_text);
}

// send msg
void message_send(struct my_msg_st* some_data){
    char buffer[BUFSIZ];
    int msgid = message_get(CLIENT_MQUEUE); 

    printf("Sending message to (CLIENT_PID %d): ", some_data->msg_key);
    fgets(buffer, BUFSIZ, stdin);

    // NOTE COME BACK TO THIS: i think since i already received from the client first, the info is still stored here so we're good
    //some_data->msg_key = 1; // this shud the client's pid here no? since im sending to that client

    strcpy(some_data->some_text, buffer);
    if (msgsnd(msgid, (void *)some_data, MAX_TEXT, 0) == -1)
    {
        fprintf(stderr, "msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
}

int main()
{
    char buffer[BUFSIZ];
    int running = 1;
    int msgid;
    struct my_msg_st some_data;
    long int msg_to_receive = 0;

    msgid = message_get(SERVER_MQUEUE);

    while (running)
    {
        message_receive(&some_data, &msg_to_receive);
        message_send(&some_data);

        if (strncmp(some_data.some_text, "end", 3) == 0 || strncmp(buffer, "end", 3) == 0){
            running = 0;
        }
    }
    if (msgctl(msgid, IPC_RMID, 0) == -1){
        fprintf(stderr, "msgctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
