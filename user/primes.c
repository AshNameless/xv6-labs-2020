#include "kernel/types.h"
#include "user/user.h"

//Pipe read, write
#define PRD 0
#define PWR 1

//Called in a process with a left pipe attached to it.
void spilt(int* p_left)
{
    //Close write end of the left pipe
    close(p_left[PWR]);

    //Get the testing divisor of this stage
    int divisor;

    //The write end of the left pipe has been closed by left process. There is no subsequent number.
    if(read(p_left[PRD], &divisor, sizeof(int)) == 0){
        close(p_left[PRD]);
        printf("\nI'm the last process generated. There is no value from left, I will exit now.\n");
        exit(0);
    
    //There are values from left. So this process need to fork another right process with a pipe connected to.
    } else {
        int p_right[2];
        pipe(p_right);

        //Current process: print the first number and pass the left numbers if divisor does not divide them.
        if(fork() == 0){
                close(p_right[PRD]);
                printf("prime %d\n", divisor);
                int num; 
                //Read from left util the write end of the left pipe closed.
                while(read(p_left[PRD], &num, sizeof(int))){
                    //divisor doesn't divide num, so num is a prime candidate.
                    if(num % divisor != 0)
                        write(p_right[PWR], &num, sizeof(int));
                }
                close(p_left[PRD]);
                close(p_right[PWR]);
                wait((int*) 0);
                exit(0);
        //New right process call spilt recusively.
        } else {
            spilt(p_right);
        }
    }
}

int main(int argc, char* argv[])
{
    int p0[2];
    pipe(p0);

    //First child
    if(fork() == 0){
        spilt(p0);
    //Parent for all processes 
    } else {
        close(p0[PRD]);
        //Generating numbers
        int i, buf[34];
        for(i = 2; i <= 35; i++)
            buf[i-2] = i;
        write(p0[PWR], &buf, 34*sizeof(int));

        close(p0[PWR]);
        wait((int*) 0);
        exit(0);
    }
    return 0;
}