#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include<sys/wait.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


#define LOKAL_PORT 80
#define BAK_LOGG 10 // Størrelse på for kø ventende forespørsler 

static void daemonizer();
static void chroot_function();
static void helloserver();
char * parseUrl(char sIn[]);
void func(int signum); 

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
    return 0;
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

                // Setter opp variabler for å lese fil
                int read_size;
                char client_message[2000];
                char inPage[128] = "";

                // Leser inn fil, parser filnavn
                if((read_size = recv(ny_sd , client_message , 2000 , 0)) > 0) {

                    char *returned_str = parseUrl(client_message);
                    strcpy(inPage, returned_str);
                  
                    // Leser filforespørsel fra klient
                    FILE* fp = fopen(returned_str, "r");
                    if(fp) {
                        
                        char buf[1024]; // Buffer på 1024

                        // Så lenge det er mer data å sende...
                        while (fgets(buf, sizeof(buf), fp) != NULL) {
                            
                            // Sender data del for del til klient
                            if (send(ny_sd, buf, strlen(buf), 0) < 0) {
                                // Error
                                
                            } else {
                                // Sender suksess
                                
                            }
                        }
                        
                        fclose(fp);
                    } else {
                        printf("Kan ikke lese fil..\n");
                        fflush(stdout);
                    }
                }

            // Sørger for å stenge socket for skriving og lesing
            // NB! Frigjør ingen plass i fildeskriptortabellen
            shutdown(ny_sd, SHUT_RDWR);
            exit(0);
        }

        else {
            signal(SIGCHLD, func); 

            close(ny_sd);
        }
      }
}

static void daemonizer() {
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
}

static void chroot_function() {
    //calling the chroot process and sets "." as the root directory
    //if error code -1 is returned, the code exits
    if(chroot(".") == -1) {
        perror("chroot");
        exit(EXIT_FAILURE);
    }
}

// Postcondition: url for fil returneres
char * parseUrl(char sIn[]) {
    
    /* data to store client query, warning, should not be big enough to handle all cases */
    char query[1024] = "";
    static char page[128] = ""; // !! Bør endres til noe annet enn static!
    char host[128] = "";
    
    // Hvis inputstring har innhold
    if (sIn > 0)
    {
        char *tok;
        char sep[] = "\r\n"; // Seperator-string
        char tmp[128];
        
        // Deler stringen inn slik at hver linje leses for seg
        tok = strtok(sIn, sep);

        // Prosesserer hver enkelt linje
        while (tok)
        {
            // Sjekker om tekst-linja inneholder "GET"
            if (1 == sscanf(tok, "GET %s HTTP/1.1", tmp))
            {
                strcpy(page, tmp); // Legger url i variable "page"
            }
            // Sjekker om lija inneholder "Host"
            else if (1 == sscanf(tok, "Host: %s", tmp))
            {
                strcpy(host, tmp); // Legger host i variable "host", ikke nødvendig i mp1
            }
            
            // Videre til neste linje
            tok = strtok(query, sep);
        }
        
        // Printer data som er mottatt
        printf("Wanted page is: %s%s\n", host, page);
        fflush(stdout);

        
    } 
    else 
    {
        /* handle the error (-1) or no data to read (0) */
    }

    return page;
}


void func(int signum) { 
    wait(NULL); 
} 

