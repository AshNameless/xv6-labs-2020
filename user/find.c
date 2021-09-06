#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"
#include "kernel/fs.h"

#define BUFSIZE 512

char* current_dir = ".";
char* parent_dir = "..";

char* get_name(char* path)
{
  char* p;
  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
  return p;
}


//Find fname in directory path. Path is guranteed to be a directory
void find_dir(char* path, char* fname)
{
  //Open the dir
  int fd;
  if((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    exit(1);
  }

  //Data of a directory in xv6 is an array consists with dirent elements.
  struct dirent de;
  struct stat st;
  char* p, *end;
  end = path + strlen(path);  //End of the dir name.
  int left = BUFSIZE - strlen(path) - 1;  //Remaining space in buf.

  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    //I just found this from user/ls.c after a toturous debugging.
    //I don't know why it works, but the entry with inum 0 is invisible to ls command.
    if(de.inum == 0)
    continue;

    //Construct the full name of the file readed from the directory.
    p = end;
    *p++ = '/';
    if(left > strlen(de.name)){
      strcpy(p, de.name);
    } else {
      fprintf(2, "find: path is too long\n");
      exit(1);
    }

    stat(path, &st);
    //The entry in dir is a normal file
    if(st.type == T_FILE){
      if(strcmp(get_name(path), fname) == 0)
      printf("%s\n", path);     
      continue;

    //The entry is a dir, recurse.
    } else if(st.type == T_DIR) {
      //DO NOT recurse over . and ..
      if(strcmp(p, current_dir) == 0 || strcmp(p, parent_dir) == 0)
      continue;

      find_dir(path, fname);

    }
    else{
        fprintf(2, "find: cannot search in device %s\n", path);
    }

  }

  close(fd);
}


int main(int argc, char* argv[])
{
  if(argc != 3) {
    fprintf(2, "Usage: find dir filename\n");
    exit(1);
  }

  //Cpoy the path string into a larger memory space.
  char buf[BUFSIZE];
  if(BUFSIZE > strlen(argv[1])){
     strcpy(buf, argv[1]);   
  } else {
    fprintf(2, "find: path is too long\n");
    exit(1);
  }
  char* path = buf, *fname = argv[2];

  //Open the file
  int fd;
  if((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    exit(1);
  }
  
  struct stat st;
  //Get file stat
  if(fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    //Do not forget to release an allocated descriptor
    close(fd);
    exit(1);
  }

  if(st.type == T_FILE){
    //Compare the file name
    if(strcmp(get_name(path), fname) == 0)
    printf("./%s\n", path);

    close(fd);

  } else if(st.type == T_DIR){
    close(fd);
    find_dir(path, fname);

  } else{
    fprintf(2, "find: cannot search in device %s\n", path);
    close(fd);
  }

  exit(0);
}
