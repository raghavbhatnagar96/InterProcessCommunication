#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    int pfds[2];
    char * filetype="*.png";
    pipe(pfds);

    if (!fork()) //child process
    {
        close(1);       /* close normal stdout */
        dup(pfds[1]);   /* make stdout same as pfds[1] */
        close(pfds[0]); /* we don't need this */
        system("ls -1 *.jpg"); // system api calls shell and runs the command and stores it in the pipe.
        //execlp("ls", "ls", "-1", "*.png", NULL);
    } 
    else //parent process
    {
        close(0);       /* close normal stdin */
        dup(pfds[0]);   /* make stdin same as pfds[0] */
        close(pfds[1]); /* we don't need this */
        execlp("wc", "wc", "-l", NULL);
    }

    return 0;
}
