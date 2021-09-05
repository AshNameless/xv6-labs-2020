#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char* argv[])
{
    //Two pipes
    int p1[2], p2[2];
    pipe(p1);
    pipe(p2);

    char c;
    int pid;

    //Child
    if(fork() == 0){
        pid = getpid();
        read(p1[0], &c, 1);
        printf("%d: received ping\n", pid);
        write(p2[1], &c, 1);
        exit(0);
    //Parent
    } else {
        c = 'Z';
        pid = getpid();
        write(p1[1], &c, 1);
        read(p2[0], &c, 1);
        printf("%d: received pong\n", pid);
        wait((int*)0);
        exit(0);
    }
    return 0;
}
