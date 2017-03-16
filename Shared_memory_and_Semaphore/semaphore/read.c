#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <sys/shm.h>

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

int main(void)
{
    key_t key;
    int semid, shmid;
    union semun arg;
    int k;
    char *data;

    if ((key = ftok("sem.c", 'J')) == -1) { //generate key
        perror("ftok");
        exit(1);
    }
    if ((semid = semget(key, 1, 0666)) == -1) { //get semaphore key
        perror("semget");
        exit(1);
    }

    if ((shmid = shmget(key, SHM_SIZE, 0644 | IPC_CREAT)) == -1) { //get shared memory segment key
        perror("shmget");
        exit(1);
    }
    
    //printf("%d %d", shmid, semid);
    /* grab the semaphore set created by seminit.c: */
    
    sem_lock(semid); //lock the semaphore
    //printf("locked");
    data = shmat(shmid, (void *)0, 0); //attach the data pointer to segment
    if (data == (char *)(-1)) {
        perror("shmat");
        exit(1);
    }
    printf("Reading data in progress... Resource locked.\n");
    printf("Data: %s\n", data);
    printf("To release resource, press return.");
    getchar();
    sem_unlock(semid); //release the lock of the semaphore
    
    return 0;
}
