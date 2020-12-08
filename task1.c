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

int main() {

  pid_t id = fork();
  if (id < 0) {

  } else if (id == 0) {

  } else {
    
  }

  return 0;
}