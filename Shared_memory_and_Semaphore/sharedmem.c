#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define SHM_SIZE 1024  /* make it a 1K shared memory segment */

int main(int argc, char *argv[])
{
    pid_t childPID;/////added
    key_t key;
    int shmid;
    char *data;
    int mode;

    if (argc != 2) {
        fprintf(stderr, "usage: shmdemo [data_to_write]\n");
        exit(1);
    }

    /* make the key: */
    /*ftok takes arguments and generates a key. This key remains same for the same set of arguments
    in which one should be a filename of a file in the present directory.*/
    if ((key = ftok("sharedmem.c", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }

    /* connect to (and possibly create) the segment. shmget returns the shmid and is responsible
    for the permissions of the segment.*/
    if ((shmid = shmget(key, SHM_SIZE, 0644 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }

    /* attach to the segment to get a pointer to it: */
    data = shmat(shmid, (void *)0, 0);
    if (data == (char *)(-1)) {
        perror("shmat");
        exit(1);
    }
    strncpy(data, "Initial", SHM_SIZE);
    printf("Initially segment contains: \"%s\"\n", data);
  
    childPID = fork();//fork 
    if(childPID >= 0) // fork was successful
    {
        if(childPID == 0) // child process
        {
            strncpy(data, argv[1], SHM_SIZE); // write the argument to the shared memory segment
            printf("Child process writes following to segment : \"%s\"\n", data);

        }
            else //Parent process
        {
            sleep(1);
            printf("Parent process reads that segment now contains: \"%s\"\n", data);

        
        }
    }
    else // fork failed
    {
        printf("\n Fork failed, quitting!!!!!!\n");
        return 1;
    }

/* shmctl controls the shared memory segment and takes care of options and permissions
It has control flags which can be used to enter commands. IPC_RMID is used to remove shared memory.*/
    if (shmctl(shmid, IPC_RMID, NULL) == -1) { 
        perror("shmctl");
        exit(1);
    }
    return 0;
}
