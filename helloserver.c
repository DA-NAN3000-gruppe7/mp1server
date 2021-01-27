/*Denne koden oppretter en socket som lytter på port 80 og så endrer brukeren av 
prosessen fra root til meg(1000) og gruppen(985). Ellers kjører den identisk som 
hallotjener.c.

OBS: bruker og gruppe må vites først. Gjør dette med shell-kommandoene:
id -u BRUKER
id -g BRUKER
bruk disse tallene i UID og GID nedenfor.
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define LOKAL_PORT 80
#define BAK_LOGG 10 // Størrelse på for kø ventende forespørsler 


//non-root user and group, default server operator-user and group
#define UID 1000
#define GID 985

static void helloserver();

int main(int argc, char* argv[]) {
    helloserver();

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


    //this is the only part of the code that is different
    //here the user and group is simply changed from root into a non-root user and group
    setuid(UID);
    setgid(GID);

    //it's unsure whether this has any impact on the prossess, but it's done either way

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