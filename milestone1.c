#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LOKAL_PORT 80
#define BAK_LOGG 10 // Størrelse på for kø ventende forespørsler 

static void daemonizer();
static void chroot_function();
static void helloserver();

int main(int argc, char* argv[]) {
    close(1);
    close(2);

    int file = open("logfile.log", O_CREAT | O_RDWR, 0644);

    dup2(file, 1);
    dup2(file, 2);
    daemonizer();
    chroot_function();
    helloserver();
    
    //printf("test");
    close(file);
}

static void helloserver() {

    struct sockaddr_in  lok_adr;
    int sd, ny_sd;
    // Setter opp socket-strukturen

    sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // For at operativsystemet ikke skal holde porten reservert etter tjenerens død
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    // Initierer lokal adresse
    lok_adr.sin_family      = AF_INET;
    lok_adr.sin_port        = htons((u_short)LOKAL_PORT);
    lok_adr.sin_addr.s_addr = htonl(         INADDR_ANY);

    // Kobler sammen socket og lokal adresse
    if ( 0==bind(sd, (struct sockaddr *)&lok_adr, sizeof(lok_adr)) )
        fprintf(stderr, "Prosess %d er knyttet til port %d.\n", getpid(), LOKAL_PORT);
    else
        exit(1);

    // Venter på forespørsel om forbindelse

    listen(sd, BAK_LOGG);

    setuid(1000);
    setgid(985);
    int nroot_sd = dup(sd);
    close(sd);

    while(1) { 
        // Aksepterer mottatt forespørsel
        ny_sd = accept(nroot_sd, NULL, NULL);    

        if(0==fork()) {

          dup2(ny_sd, 1); // redirigerer socket til standard utgang

          printf("HTTP/1.1 200 OK\n");
          printf("Content-Type: text/plain\n");
          printf("\n");
          printf("Hallo klient!\n");

          fflush(stdout);

          // Sørger for å stenge socket for skriving og lesing
          // NB! Frigjør ingen plass i fildeskriptortabellen
          shutdown(ny_sd, SHUT_RDWR);
          exit(0);
        }

        else
          close(ny_sd);
      }
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
        perror("fork");
        exit(EXIT_FAILURE);
    //exits the parent process. Now only the child is running
    if (pid > 0)
        exit(EXIT_SUCCESS);

    //creates a session from the child process and assigns it the lead session ID
    //if this process fails(returns > 0) the child process terminates 
    if (setsid() < 0)
        perror("setsid");
        exit(EXIT_FAILURE);

    //this is to ignore the SIGHUP signal which is sent whenever a session leader process is terminated
    signal(SIGHUP, SIG_IGN);

    //forks the process again to ensure that it's no longer a session leader and cannot be accessed through an available terminal
    pid = fork();

    if (pid < 0)
        perror("fork");
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);
}

static void chroot_function()
{
    //calling the chroot process and sets "." as the root directory
    //if error code -1 is returned, the code exits
    if(chroot(".") == -1) {
        perror("chroot");
        exit(EXIT_FAILURE);
    }

}
