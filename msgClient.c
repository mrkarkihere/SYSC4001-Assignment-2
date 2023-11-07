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
    char some_text[MAX_TEXT];
};

// msgget() call with all the stuff so no repeats
int message_get(int QUEUE_ID)
{

    int msgid = msgget((key_t)QUEUE_ID, 0666 | IPC_CREAT);

    if (msgid == -1)
    {
        fprintf(stderr, "msgget failed with error: % d\n", errno);
        exit(EXIT_FAILURE);
    }

    return msgid;
}

// send msg
void message_send(struct my_msg_st *some_data)
{
    char buffer[BUFSIZ];
    int msgid = message_get(SERVER_MQUEUE);

    printf("Enter some text (CLIENT_PID %d): ", some_data->msg_key);
    fgets(buffer, BUFSIZ, stdin);
    strcpy(some_data->some_text, buffer);
    if (msgsnd(msgid, (void *)some_data, MAX_TEXT, 0) == -1)
    {
        fprintf(stderr, "msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
}

// receive msg
void message_receive(struct my_msg_st* some_data)
{

    int msgid = message_get(CLIENT_MQUEUE);

    if (msgrcv(msgid, (void *)some_data, BUFSIZ, some_data->msg_key, 0) == -1)
    {
        fprintf(stderr, "msgrcv failed with error: % d\n", errno);
        exit(EXIT_FAILURE);
    }
    printf("(SERVER) wrote: %s", some_data->some_text);
}

int main()
{
    int running = 1;
    struct my_msg_st some_data;
    int msgid;
    char buffer[BUFSIZ];

    msgid = message_get(SERVER_MQUEUE);
    some_data.msg_key = getpid();
    while (running)
    {
        message_send(&some_data);
        message_receive(&some_data);
        if (strncmp(some_data.some_text, "end", 3) == 0 || strncmp(buffer, "end", 3) == 0){
            running = 0;
        }
    }
    exit(EXIT_SUCCESS);
}
