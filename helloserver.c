#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define LOKAL_PORT 55556
#define BAK_LOGG 10 // Størrelse på for kø ventende forespørsler 

static void daemonizer();
static void chroot_function();


int main(int argc, char* argv[]) {
    //daemonizer();
    //chroot_function();
    
    struct sockaddr_in  lok_adr;
    int sd, ny_sd;
    pid_t pid;
    // Setter opp socket-strukturen

    sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // For at operativsystemet ikke skal holde porten reservert etter tjenerens død
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    // Initierer lokal adresse
    lok_adr.sin_family      = AF_INET;
    lok_adr.sin_port        = htons(80); 
    lok_adr.sin_addr.s_addr = htonl(         INADDR_ANY);

    // Kobler sammen socket og lokal adresse
    if ( 0==bind(sd, (struct sockaddr *)&lok_adr, sizeof(lok_adr)) )
        fprintf(stderr, "Prosess %d er knyttet til port %d.\n", getpid(), ntohs(lok_adr.sin_port));
    else
        exit(1);

    // Venter på forespørsel om forbindelse
    listen(sd, BAK_LOGG);


    while(1) { 

        // Aksepterer mottatt forespørsel
        ny_sd = accept(sd, NULL, NULL);   

        pid = fork();

        if (pid < 0)
            exit(EXIT_FAILURE);

        if (pid > 0)
            close(ny_sd);
            exit(EXIT_SUCCESS);

    

        dup2(ny_sd, 1); // redirigerer socket til standard utgang
        close(ny_sd);

        setuid();
        setgid();

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


    
  
  return 0;
}