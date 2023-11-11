#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sem.h>
#include "semun.h"

static int set_semvalue(void);
static void del_semvalue(void);
static int semaphore_p(void);
static int semaphore_v(void);
static int sem_id;

int main()
{

    /* PROBLEM STATEMENT:

    P1 - parent process
    P2 - child process

    S1 - printf(“I’m possible!\n”);
    S2 - printf(“Smile while you still have teeth. :) :)\n”);

    GOAL: P1 has to wait until P2 finishes statement S2 before it can execute statement S1

    OUTPUT:
    S2
    S1

    REQUIREMENT: semaphor needs to be initially 0 so i can wait() for the child to signal
    */

    // summary params: key, # of sems, permissions
    sem_id = semget((key_t)(1234), 1, 0666 | IPC_CREAT);
    set_semvalue(); // set default values; set up semaphor

    // ill model this using fork
    pid_t child_pid;
    child_pid = fork();

    switch (child_pid)
    {
    case -1:
        perror("forking failed\n");
        exit(EXIT_FAILURE);
    // child
    case 0:
        printf("Smile while you still have teeth. :) :)\n"); // S2
        sleep(1);
        semaphore_v();
        break;
    // parent
    default:
        semaphore_p();
        printf("I’m possible!\n"); // S1
        del_semvalue(); // destroy semaphore
        break;
    }
    return EXIT_SUCCESS;
}

static int set_semvalue(void)
{
    union semun sem_union;
    sem_union.val = 0;// 0 = force to wait for a signal initially
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1)
        return (0);
    return (1);
}
static void del_semvalue(void)
{
    union semun sem_union;
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore\n");
}
static int semaphore_p(void)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "semaphore_p failed\n");
        return (0);
    }
    return (1);
}
static int semaphore_v(void)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "semaphore_v failed\n");
        return (0);
    }
    return (1);
}