/**
 * @file consumer.c
 * @author Arun Karki
 * @date 2023-11-12
 * 
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
void message_send(struct msg_data* data, char* request){
    int msgqid = message_get();

    sprintf(data->message, "%s", request); // this is the request message being sent to the server

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
    return (key_t) key;
}

// return the shared memory key generated by server
key_t get_shm_key(struct msg_data* data, char* request){
    data->msg_type = SERVER_MSG_TYPE; // send a type 1 for server to read
    sprintf(request, "REQUEST;%s", SH_MEM_REQ_MSG);
    message_send(data, request);

    // shared memory key received from server
    data->msg_type = getpid(); // receive to client that sent the request
    return message_receive(data);
}

// return the semaphore key generated by server
key_t get_sem_key(struct msg_data* data, char* request){
    data->msg_type = SERVER_MSG_TYPE; // send a type 1 for server to read
    sprintf(request, "REQUEST;%s", SEM_REQ_MSG);
    message_send(data, request);

    // semaphore key received from server
    data->msg_type = getpid(); // receive to client that sent the request
    return message_receive(data);
}

// get shared memory region
key_t get_shared_mem(key_t *shm_key){
    key_t shm_id = shmget(*shm_key, sizeof(struct shm_buffer) * NUM_BUFFER, SHM_PERMISSION);
    if (shm_id == -1){
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }
    return shm_id;
}

// attach to the shared memory region
void attach_shared_mem(key_t* shm_id, void** shm_ptr){
    *shm_ptr = shmat(*shm_id, (void *)0, 0);
    if (*shm_ptr == (void *)-1){
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }
}

/* SEMAPHORE Functions */
// wait()
static int semaphore_p(key_t* sem_id, int sem_num){
    struct sembuf sem_b;
    sem_b.sem_num = sem_num;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(*sem_id, &sem_b, 1) == -1){
        fprintf(stderr, "semaphore_p failed\n");
        return (0);
    }
    return (1);
}
// signal()
static int semaphore_v(key_t* sem_id, int sem_num){
    struct sembuf sem_b;
    sem_b.sem_num = sem_num;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(*sem_id, &sem_b, 1) == -1){
        fprintf(stderr, "semaphore_v failed\n");
        return (0);
    }
    return (1);
}

// maybe use command line arguments for file name -> good idea
int main(int argc, char *argv[]){

    printf("\n---------| PRODUCER |---------\n");

    char output_file_name[BUFFER_SIZE] = "output.txt"; 
    int output_file;   

    int msgqid; // queue message id
    struct msg_data data; // queue data struct
    char request; // what type of request am i making, ex. 'sh_key'

    key_t shm_key, sem_key; // shared memory and semaphore key generated by server
    key_t shm_id, sem_id; // shared memory region and semaphore id

    // the shared memory buffer pointer; attach this
    struct shm_buffer* shared_memory;
    void *shm_ptr = (void *) 0; // point to the shared region    

    // get message queue
    msgqid = message_get(); // get message queue id

    printf("\noutput file: %s\n", output_file_name);

    /* REQUEST SHARED MEMORY KEY */
    shm_key = get_shm_key(&data, &request);
    shm_id = get_shared_mem(&shm_key); //create or get shared memory
    attach_shared_mem(&shm_id, &shm_ptr); // attach to ptr
    shared_memory = (struct shm_buffer *) shm_ptr; // assign to buffer

    printf("Shared Memory Key: %d\n", shm_key);
    printf("Shared Memory ID: %d\n", shm_id);
    printf("Shared Memory Address: %p\n", shm_ptr);    

    /* REQUEST SEMAPHORE KEY */
    sem_key = get_sem_key(&data, &request);
    sem_id = semget(sem_key, NUM_SEMAPHORE, SEM_PERMISSION);

    printf("\nSemaphore Key: %d\n", sem_key);
    printf("Semaphore ID: %d\n\n", sem_id);

    // write to output file and read from buffer
    output_file = open(output_file_name, O_WRONLY);

    if(output_file == -1){
        perror("error opening output file");
        exit(EXIT_FAILURE);
    }

    /* GET from buffer(s) and WRITE to output file */
    int bytesWritten = 0;
    for(int i = 0; i < NUM_BUFFER; i++){
        
        struct shm_buffer* curr_buff = &shared_memory[i]; // current buffer

        if(curr_buff->count > 0){ // there's stuff to retrieve
        
            // wait()
            semaphore_p(&sem_id, 1); // wait on Full (N)
            semaphore_p(&sem_id, 2); // wait on Mutex (S)

            // empty the buffer
            curr_buff->count = 0;

            // verify sequenceNumber
            if(bytesWritten != curr_buff->sequence_number)
                perror("ERROR: unexpected sequence number\n");

            // read contents of buffer[i] and put into output file
            bytesWritten += write(output_file, curr_buff->message, strlen(curr_buff->message));

            // signal()
            semaphore_v(&sem_id, 2); // signal on Mutex (S)
            semaphore_v(&sem_id, 0); // signal on Empty (E)
        }
        else{
            printf("Buffer [%d] is empty\n", i);
        }
    }

    printf("\nFinished writing to %s\n", output_file_name);
    printf("Received: %d bytes in total.\n$$$$", bytesWritten);

    // clean up
    close(output_file); // close the output file
    semaphore_v(&sem_id, 3); // signal on Destroy
    message_send(&data, TERMINATION_CODE); // send server to destroy
    msgctl(msgqid, IPC_RMID, NULL); // close queue

    return 0;
}