#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"
#include "kernel/fs.h"

void find(char* path, char* fname)
{
  //Open the file(dir is also a file in unix-like systems).
  int fd;
  if((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s", path);
    exit(1);
  }
  
  struct stat st;
  struct dirent dir;

  //Get file stat
  if(fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    //Do not forget to release an allocated descriptor
    close(fd);
    exit(1);
  }
  switch(st.type)
  {
  case T_FILE:

  }
}

int main(int argc, char* argv[])
{
  if(argc != 3) {
    fprintf(2, "Usage: find dir file");
    exit(1);
  }
  find(argv[1], argv[2]);
  exit(0);
}
