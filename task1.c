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

  int ret, file, status;
  int fd[2], origin[2];

  pipe(origin);
  dup2(0, origin[0]);
  dup2(1, origin[1]);

  pipe(fd);

  file = open(argv[3], O_RDONLY, 0644);
  dup2(file, 0);
  close(file);

  pid_t id = fork();
  pid_t pid;
  if (id < 0) {
    perror("fork error: ");
    exit(1);
  } else if (id == 0) {
    dup2(fd[1], 1);
    close(fd[0]);
    close(fd[1]);

    ret = execlp(argv[1], argv[1], NULL);
    printf("%d\n", ret);
    perror("execlp 1 error: ");

  } else wait(&status);

  if (status != 0) {
    pid = fork();
    if (pid < 0) {
      perror("fork error: ");
      exit(1);
    } else if (pid == 0) {
      dup2(fd[1], 1);
      close(fd[0]);
      close(fd[1]);

      execlp(argv[2], argv[2], NULL);
      perror("execlp 2 error: ");
      //exit(3);
    } else {
      wait(NULL);
    }
  }
  
  dup2(fd[0], 0);
  close(fd[0]);
  close(fd[1]);
  dup2(origin[1], 1);

  id = fork();
  if (id < 0) {
    perror("fork error: ");
    exit(4);
  } else if (id == 0) {
    execlp(argv[4], argv[4], NULL);
    perror("execlp 4 error: ");
    exit(5);
  } else {
    wait(NULL);
  }

  dup2(origin[0], 0);

  return 0;
}