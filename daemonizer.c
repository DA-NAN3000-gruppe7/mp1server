/*Denne koden oppretter en demonisert prosess som foregår i bakgrunnen.
For å bruke denne koden til nettleseren kopierer man bare den inn i koden
til prosessen man ønsker å demonisere og kalle daemonizer() før resten av koden kjører

For å avlsutte prosessen skriver man bare "killall daemonizer.o"
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static void daemonizer();

//main function
int main(int argc, char* argv[]) {
    daemonizer();

}

static void daemonizer()
{
    //create a process ID variable
    pid_t pid;

    //creates a fork of the process, a parent and a child
    //the code should run identical in practice for both child and parent
    //but they each have a different value assigned to the pid variable
    //a child has pid = 0 and the parent pid > 0. a pid < 0 means the forking failed
    //this is so that the process can run in the background and so that it is not a process group leader
    pid = fork();


    if (pid < 0)
        exit(EXIT_FAILURE);
    //exits the parent process. Now only the child is running
    if (pid > 0)
        exit(EXIT_SUCCESS);

    //creates a session from the child process and assigns it the lead session ID
    //if this process fails(returns > 0) the child process terminates 
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    //this is to ignore the SIGHUP signal which is sent whenever a session leader process is terminated
    signal(SIGHUP, SIG_IGN);

    //forks the process again to ensure that it's no longer a session leader and cannot be accessed through an available terminal
    pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    //this code here is just to test if the daemonizer works.
    //by doing cat log.txt, you should see if it writes "Test" to the file every 1 sec
    ///*
    FILE *f = fopen("log.txt", "w");
    if(f == NULL)
        exit(EXIT_FAILURE);

    while (1)
    {
        sleep(1);
        fprintf(f, "Test\n");
        fflush(f);
    }
    fclose(f);
    //*/
}