/*Denne koden setter cwd(current word directory) som rot for prosessen slik at man ikke har tillgang til filer ovenfor cwd.
For å bruke denne koden til nettleseren kopierer man bare den inn i koden
til prosessen man ønsker å endre roten til cwd og kalle chroot_function() før resten av koden kjører
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void chroot_function();

//main function
int main(int argc, char* argv[]) {
	chroot_function();

}

static void chroot_function()
{
	//this only gets the cwd the process is running in
	//this code can be commented/removed in real applications
	///*
    char buf[1024];
    getcwd(buf, 1024);
    printf("current working directory: %s\n", buf);
    //*/

    //calling the chroot process and sets "." as the root directory
    //if error code -1 is returned, the code exits
    if(chroot(".") == -1) {
        perror("chroot");
        exit(EXIT_FAILURE);
    }
    ///*
    getcwd(buf, 1024);
    printf("current working directory: %s\n", buf);
    //*/


}