#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>

#define MAX_RETRIES 10
#define SHM_SIZE 1024

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

/* sem_lock takes semid as argument and unlocks the semaphore that is not locked.
sem_op =-1 for this which tells the semop api to lock the semaphore.*/
void sem_lock(int semid)
{
     /* structure for semaphore operations.   */
    struct sembuf sem_op;

    /* wait on the semaphore, unless it's value is non-negative. */
    sem_op.sem_num = 0;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    if (semop(semid, &sem_op, 1) == -1) {
        printf("couldnt lock");
        perror("semLock");
        exit(1);
    }
    //semop(sem_set_id, &sem_op, 1);
}

/* sem_unlock takes semid as argument and unlocks the semaphore that has been locked previously.
sem_op =1 for this which tells the semop api to unlock the semaphore.*/
void sem_unlock(int semid)
{
    /* structure for semaphore operations.   */
    struct sembuf sem_op;

    /* signal the semaphore - increase its value by one. */
    sem_op.sem_num = 0;
    sem_op.sem_op = 1;   /* <-- Comment 3 */
    sem_op.sem_flg = 0;
    if (semop(semid, &sem_op, 1) == -1) {
        printf("couldnt lock");
        perror("semUnlock");
        exit(1);
    }
    //semop(sem_set_id, &sem_op, 1);
}

/*
** initsem() -- more-than-inspired by W. Richard Stevens' UNIX Network
** Programming 2nd edition, volume 2, lockvsem.c, page 295.
*/
int initsem(key_t key, int nsems)  /* key from ftok() */
{
    int i;
    union semun arg;
    struct semid_ds buf;
    struct sembuf sb;
    int semid;

    semid = semget(key, nsems, IPC_CREAT | IPC_EXCL | 0666);

    printf("Semget() returned %d \n",semid);

    if (semid >= 0) 
    { /* we got it first */
        sb.sem_op = 1; sb.sem_flg = 0;
        arg.val = 4;

        printf("press return\n"); getchar();

        for(sb.sem_num = 0; sb.sem_num < nsems; sb.sem_num++) 
        { 
            /* do a semop() to "free" the semaphores. */
            /* this sets the sem_otime field, as needed below. */
            if (semop(semid, &sb, 1) == -1) 
            {
                int e = errno;
                semctl(semid, 0, IPC_RMID); /* clean up */
                errno = e;
                return -1; /* error, check errno */
            }
        }
    } 
    else if (errno == EEXIST) 
    { /* someone else got it first */
        int ready = 0;

        semid = semget(key, nsems, 0); /* get the id */
        if (semid < 0) return semid; /* error, check errno */

        /* wait for other process to initialize the semaphore: */
        arg.buf = &buf;
        for(i = 0; i < MAX_RETRIES && !ready; i++) 
        {
            semctl(semid, nsems-1, IPC_STAT, arg);
            if (arg.buf->sem_otime != 0) 
            {
                ready = 1;
            } 
            else 
            {
                sleep(1);
            }
        }
        if (!ready) {
            errno = ETIME;
            return -1;
        }
    } else {
        return semid; /* error, check errno */
    }

    return semid;
}


int main(void)
{
    key_t key;
    int semid;
    int shmid;
    char *data;
    struct sembuf sb;
    union semun arg;

    sb.sem_num = 0;
    sb.sem_op = -1;  /* set to allocate resource */
    sb.sem_flg = SEM_UNDO;

    if ((key = ftok("sem.c", 'J')) == -1) {
        perror("ftok");
        exit(1);
    }
    printf("ftok() returned \n");
    /* grab the semaphore set created by seminit.c: */
    if ((semid = initsem(key, 1)) == -1) {
        perror("initsem");
        exit(1);
    }

    printf("initsem returned \n");

    if ((shmid = shmget(key, SHM_SIZE, 0644 | IPC_CREAT)) == -1) { //create the shared memory segment
        perror("shmget");
        exit(1);
    }

    data = shmat(shmid, (void *)0, 0); /* attach to the segment to get a pointer to it: */ 
    if (data == (char *)(-1)) {
        perror("shmat");
        exit(1);
    }

    strncpy(data, "initial", SHM_SIZE);
    printf("To destroy semaphore, press return.");
    getchar();

    /*semctl is used to control the permission and other characteristics of semaphore. IPC_RMID
    is used to destroy the semaphore*/
    if (semctl(semid, 0, IPC_RMID, arg) == -1) { 
        perror("semctl");
        exit(1);
    }
    return 0;
}
