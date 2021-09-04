#include "user/user.h"


int main(int argc, char* argv[])
{
    //Sleep command should have only 2 arguments
    if(argc != 2){
        fprintf(2, "Usage: sleep number\n");
        exit(1);
    }
    
    //Compute the number
    int n;
    n = atoi(argv[1]);

    if(sys_sleep(n))
        exit(1);
    exit(0);
}





