#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

#define BUFMAX 256


int main(int argc, char* argv[])
{
    if(argc == 1){
        fprintf(2, "Usage: xargs command\n");
        exit(1);
    }

    //Get the command to be executed.
    char* command[MAXARG];
    int i, arg_count = argc - 1;
    for(i = 1; i < argc; i++)
    command[i-1] = argv[i];

    //Read from stdin, assemble arguments into command. If a '\n' appeared, execute the command.
    char buf[BUFMAX], c;
    int flag = 0;   //flag whether the input is empty or not.
    int arg_start = 0;
    i = 0;
    while(1){
        if(arg_count >= MAXARG){
            fprintf(2, "xargs: too many arguments\n");
            exit(1);
        }
        //End of file.
        if(read(0, &c, 1) == 0){
            if(flag == 1){
                buf[i] = '\0';
                command[arg_count++] = &buf[arg_start];
            }
            command[arg_count] = 0;
            exec(argv[1], command);
        }

        //Execute it, reset variables for new commands.
        if(c == '\n'){
            buf[i] = '\0';
            command[arg_count++] = &buf[arg_start];
            command[arg_count] = 0;

            //Child
            if(fork() == 0){
                for(i=0;i<arg_count;i++)
                exec(argv[1], command);
            //Continue reading.
            } else {
                flag = 1;
                i = 0;
                arg_start = 0;
                arg_count = argc - 1;
                wait((int*) 0);
                continue;
            }

        //The end of a argument.
        } else if(c == ' '){
            command[arg_count++] = &buf[arg_start];
            buf[i++] = '\0';
            arg_start = i;
        //Normal character.
        } else {
            buf[i++] = c;
        }
    }

    return 0;
}