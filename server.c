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
void message_send(){
    int msgqid = message_get();
    int res = msgsnd(msgqid, );
}

int main(){


    return 0;
}
