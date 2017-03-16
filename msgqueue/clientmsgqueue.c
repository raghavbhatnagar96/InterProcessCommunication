#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct my_msgbuf { //datastructure of the message.
    long mtype;
    char mtext[200];
};

int main(void)
{
    struct my_msgbuf buf;
    int msqid; //message queue id that is used to access it.
    key_t key; //unique key to identify particular queue using msgget()

    /*ftok takes arguments and generates a key. This key remains same for the same set of arguments
    in which one should be a filename of a file in the present directory.*/
    
    if ((key = ftok("clientmsgqueue.c", 'C')) == -1) {
        perror("ftok");
        exit(1); //exit(1) is exit due to unsuccessful termination while exit(0) is succesful program termination.
    }

    /*msgget() is used for both initializing new message queue and to get id of existing message 
    queue.Values passed are the uniques key, permissions and the control flag value. IPC_create is 
    used to create new queue if one doesn't exist and return the queue id. If the queue exists, the id 
    of the queue is returned.*/

    if ((msqid = msgget(key, 0644)) == -1) {
        perror("msgget");
        exit(1);
    }
    
    printf("Enter text:\n");

    buf.mtype = getpid(); //in this case, mtype is used to distinguish between the clients.
    printf("<number> <operator(+,-,*,/,^ only)> <number>:\n");
    while(fgets(buf.mtext, sizeof buf.mtext, stdin) != NULL) {
        int len = strlen(buf.mtext);

        /* ditch newline at end, if it exists */
        if (buf.mtext[len-1] == '\n') buf.mtext[len-1] = '\0';

    /*msgsnd is used to send the message to the end of the queue and takes arguments queue id,
    message, size and flag*/
        if (msgsnd(msqid, &buf, len+1, 0) == -1) /* +1 for '\0' */
            perror("msgsnd");
        if (strcmp(buf.mtext, "exit")==0){
            perror("msgsnd");
            exit(1);
        }

    }
    /*msgctl() function is used to alter permissions of the queue along with other characteristics.
    It is also used to destroy the queue by using flag IPC_RMID as used here.*/
    if (msgctl(msqid, IPC_RMID, NULL) == -1)
    {
        perror("msgctl");
        exit(1);
    }

    return 0;
}
