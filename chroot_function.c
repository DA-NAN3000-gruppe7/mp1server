#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void chroot_function();


int main(int argc, char* argv[]) {
	chroot_function();

}

static void chroot_function()
{

    char buf[1024];
    getcwd(buf, 1024);
    printf("current working directory: %s\n", buf);

    if(chroot(".") == -1) {
        perror("chroot");
        exit(EXIT_FAILURE);
    }
    getcwd(buf, 1024);
    printf("current working directory: %s\n", buf);


}