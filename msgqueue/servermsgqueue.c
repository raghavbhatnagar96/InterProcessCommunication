#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int operate(int, char, int);

struct my_msgbuf { //datastructure of the message.
    long mtype;
    char mtext[200];
};

int main(void)
{
    struct my_msgbuf buf;
    FILE *outputFile; //File pointer used to write to file.
    int num1, num2;
    char operator;
    int msqid; //message queue id that is used to access it.
    char *token;
    key_t key; //unique key to identify particular queue using msgget()
    int result;
    outputFile = fopen("output.dat", "w+");
    fclose(outputFile);

    /*ftok takes arguments and generates a key. This key remains same for the same set of arguments
    in which one should be a filename of a file in the present directory.*/

    if ((key = ftok("clientmsgqueue.c", 'C')) == -1) { 
        perror("ftok");
        exit(1);
    }

    /*msgget() is used for both initializing new message queue and to get id of existing message 
    queue.Values passed are the uniques key, permissions and the control flag value. IPC_create is 
    used to create new queue if one doesn't exist and return the queue id. If the queue exists, the id 
    of the queue is returned.*/
    
    if ((msqid = msgget(key, 0644 | IPC_CREAT)) == -1) { /* connect to the queue */
        perror("msgget");
        exit(1);
    }
    
    printf("Server: ready to receive messages!\n");
    printf("<ClientID>: <Request> = <Result>\n");

    for(;;) { /* Server never quits! */
        
        /*msgrcv is used to find the newest entry in the message queue.
        it takes arguments queue id, variable, size and flags.*/
        if (msgrcv(msqid, &buf, sizeof buf.mtext, 0, 0) == -1) 
        {
            perror("msgrcv");
            exit(1);
        }
        
        /*message recieved is parsed using the function strtok that breaks the string into tokens
        around whitespaces as seen below to give us the numbers and the operations*/
        token = strtok(buf.mtext, " ");
        if(token==NULL) // chech to see if token is null. If it is, the input is not proper.
        {
        	printf("%li: Empty string recieved\n", buf.mtype);
          outputFile = fopen("output.dat", "a+");
          fprintf(outputFile, "%li: Empty string recieved\n", buf.mtype);
          fclose(outputFile);
        }
        else
        {
        	num1 = atoi(token); //if input is proper, the first argument is the first number
        	{
        		token = strtok(NULL, " ");
        		if(token==NULL) //cheching if second argument is null
        		{
        			printf("%li: Invalid input\n", buf.mtype);
              outputFile = fopen("output.dat", "a+"); //write in file
              fprintf(outputFile, "%li: Invalid input\n", buf.mtype);
              fclose(outputFile);
        		}
        		else
        		{
        			operator=token[0];
        			token = strtok(NULL, " ");
        			if(token==NULL)//checking if 3rd argument is null.
	        		{
	        			printf("%li: Invalid input\n", buf.mtype);
                outputFile = fopen("output.dat", "a+");
                fprintf(outputFile, "%li: Invalid input\n", buf.mtype);
                fclose(outputFile);
	        		}
        			else
        			{
        				num2 = atoi(token);//final number is 3rd argument.
        				result = operate(num1, operator, num2); // get result from mathematical engine.
				        if(result!=-9999)
				        {
				        	printf("%li: %d %c %d = %d\n", buf.mtype, num1, operator, num2, result);			        
                  outputFile = fopen("output.dat", "a+");
                  fprintf(outputFile, "%li: %d %c %d = %d\n", buf.mtype, num1, operator, num2, result);//enter result in file
                  fclose(outputFile);
                }
        			}
        		}
        	}
        }
       	

    }
    return 0;
}

/* Operation that uses mathematical engine to return result */

int operate(int num1, char operator, int num2)
{
   
   int result, i;
   if(operator=='+')
   {
      return num1 + num2;
   }
   else if(operator=='-')
   {
      return num1 - num2;
   }
   else if(operator=='*')
   {
      return num1 * num2;
   }
   else if(operator=='/')
   {
      return num1 / num2;
   }
   else if(operator=='^')
   {
      result = 1;
      for(i=num2;i>0;i--)
      {
        result=result*num1;
      }
      return result;
   }
   else
   {
      printf("invalid operator\n");
      return -9999;
   }
   
}
