#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static void daemonizer();


int main(int argc, char* argv[]) {
    daemonizer();

}

static void daemonizer()
{
    pid_t pid;

    pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    signal(SIGHUP, SIG_IGN);

    pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);


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
}