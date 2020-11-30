#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>

char *word;
char **arg_v;
int c, num=0;

int enterCommand(){
  int i=0, redirect=-1, file;
  while ((c != '\n') && (c != EOF)) {

    word = malloc(sizeof(char *));
    while ((c == ' ') || (c == '\t')) c = getchar();

    if (c == '\n') break;
    i = 0;

    if ( c == '<' ) {redirect = 0; c = getchar();}
    else if ( c == '>') {
      redirect = 1;
      if ( (c = getchar()) == '>') {
        redirect = 2;
        c = getchar();
      }
    }

    if (c != '"') {

      while ((c != ' ') && (c != '\t') && (c != '\n') && (c != '|') && (c != '<') & (c != '>')) {
        word[i] = c;
        i++;
        word = realloc(word, i * sizeof(char *));
        c = getchar();
      }
      word[i] = '\0';
    } else {

      while ((c = getchar()) != '"') {
        word[i] = c;
        i++;
        word = realloc(word, i * sizeof(char *));
      }
      word[i] = '\0';
      if (c == '"') c = getchar();
    }

    switch (redirect) {
      case 0:
        file = open(word, O_RDONLY, 0644);
        dup2(file, 0);
        close(file);
        break;
      case 1:
        file = open(word, O_RDWR | O_TRUNC | O_CREAT, 0644);
        dup2(file, 1);
        close(file);
        break;
      case 2:
        file = open(word, O_RDWR | O_APPEND, 0644);
        dup2(file, 1);
        close(file);
        break;
      case -1:
        if (word[0] != '\0') {
          arg_v = realloc(arg_v, (num + 1) * sizeof(char **));
          arg_v[num] = malloc((strlen(word) + 1) * sizeof(char *));
          strcpy(arg_v[num], word);
          num++;
        }

    }
    free(word);
    if (c == '|') break;
  }
  return redirect;
}

int main() {

  int i=0, fd[2], origin[2], redir=-1;
  pid_t id;

  pipe(origin);
  dup2(0, origin[0]);
  dup2(1, origin[1]);

  printf(" >>> ");
  c = getchar();

  while ( c != EOF) {

    arg_v = malloc(sizeof(char **));
    arg_v[0] = NULL;
    num = 0;

    redir = enterCommand();

    if (arg_v[0]) {
      if ((c == '\n') || (c == EOF)) {
        if (redir <= 0) dup2(origin[1], 1);
        if (strcmp(arg_v[0], "exit") == 0) {
          for (i = 0; i < num; i++) free(arg_v[i]);
          free(arg_v);
          break;
        } else {
          arg_v = realloc(arg_v, (num + 1) * sizeof(char **));
          arg_v[num] = NULL;
          num++;
          if (strcmp(arg_v[0], "cd") == 0) {
            if (arg_v[1] == NULL) {
              char home_path[PATH_MAX];
              strcpy(home_path, getenv("HOME"));
              chdir(home_path);
            } else if (chdir(arg_v[1]) == -1) perror("chdir error");
          } else {

            id = fork();
            if (id < 0) {
              perror("fork error");
              exit(4);
            } else if (id == 0) {
              execvp(arg_v[0], arg_v);
              perror("execvp error");
              exit(1);
            } else {
              wait(NULL);
            }
          }
        }
        dup2(origin[0], 0);
        dup2(origin[1], 1);
        printf(" >>> ");
      } else if (c == '|'){
        arg_v = realloc(arg_v, (num + 1) * sizeof(char **));
        arg_v[num] = NULL;
        num++;
        if (strcmp(arg_v[0], "cd") && strcmp(arg_v[0], "exit")){
          pipe(fd);
          id = fork();
          if (id < 0) {
            perror("fork error 2");
            exit(5);
          } else if (id == 0){
            dup2(fd[1], 1);
            close(fd[0]);
            close(fd[1]);
            execvp(arg_v[0], arg_v);
            perror("exec error 2");
            exit(6);
          }
          dup2(fd[0], 0);
          close(fd[0]);
          close(fd[1]);
        } else fprintf(stderr, "Incorrect use of commands (cd/exit) in pipe");
        while(wait(NULL) != -1);
      }
    }

    for (i = 0; i < num; i++) free(arg_v[i]);
    free(arg_v);

    c = getchar();
  }

  return 0;
}