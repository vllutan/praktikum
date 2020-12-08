#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>

int main(int argc, char **argv) {

  for (int i=0; i<argc; i++){
    printf("%d %s\n", i, argv[i]);
  }

  int ret, file;
  int fd[2];
  pipe(fd);

  file = open(argv[3], O_RDONLY, 0644);
  dup2(file, 0);
  close(file);

  pid_t id = fork();
  if (id < 0) {
    perror("fork error: ");
    exit(1);
  } else if (id == 0) {

    ret = execlp(argv[1], argv[1], NULL);
    printf("%d\n", ret);
    perror("execlp 1 error: ");
    //exit(2);
    if (ret == -1){
      execlp(argv[2], argv[2], NULL);
      perror("execlp 2 error: ");
      exit(3);
    }

  } else {
    wait(NULL);
  }

  return 0;
}