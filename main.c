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

char *word;
char **arg_v;
int c, ch, num=0, if_back=0, if_pipe=0, redir_inp=0, redir_outp=0;

int enterCommand(){
  int i=0, redirect=-1, file;
  while ((c != '\n') && (c != EOF) && (c != ';')) {

    word = malloc(sizeof(char *));
    while ((c == ' ') || (c == '\t')) c = getchar();

    if (c == '\n') break;
    i = 0;

    if ( c == '<' ) {redirect = 0; redir_inp = 1; c = getchar();}
    else if ( c == '>') {
      redirect = 1; redir_outp = 1;
      if ( (c = getchar()) == '>') {
        redirect = 2;
        c = getchar();
      }
    }

    if (c != '"') {

      while ((c != ' ') && (c != '\t') && (c != '\n') && (c != '|') && (c != '<') && (c != '>') && (c != ';')) {
        if (if_back == 1) {
          if_back = 2;
          break;
        } else if (c == '&') {
          if_back = 1;
          c = getchar();
        } else {
          word[i] = c;
          i++;
          word = realloc(word, i * sizeof(char *));
          c = getchar();
        }
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

    while ((c == ' ') || (c == '\t')) c = getchar();

    if(if_back && (c != '\n') && (c != ';')) {
      if_back = 3;
      fprintf(stderr, "& error: not at the end\n");
      exit(8);
    }

    if(if_back < 2) {
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
    }
    free(word);
    if (c == '|') break;
  }
  return redirect;
}

void KillProc(int sig){
  exit(0);
}

int main() {

  int i=0, fd[2], origin[2], status, back_proc=0, back_released=1;
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
    if_back = 0;
    redir_outp=0; redir_inp=0;

    signal(SIGINT, SIG_IGN);

//  -------------------- PROCESSING BACK PROCESSES --------------------------------------------

    while((id = waitpid(-1, &status, WNOHANG)) > 0){
      if(WIFEXITED(status)){
        printf("[%d] finished well with status %d\n\n", back_released, WEXITSTATUS(status));
        back_released++;
        back_proc--;
      } else if (WIFSIGNALED(status)){
        printf("[%d] finished idk how with status %d\n\n", back_released, WTERMSIG(status));
        back_released++;
        back_proc--;
      } else if (WIFSTOPPED(status)){
        printf("[%d] finished badly with status %d\n\n", back_released, WSTOPSIG(status));
        back_released++;
        back_proc--;
      }
      if(back_proc == 0) back_released = 1;
    }



    enterCommand();

//   --------------- PROCESSING COMMAND in IF cycle ---------------------------------------

    if (arg_v[0] && (if_back < 2)) {
      arg_v = realloc(arg_v, (num + 1) * sizeof(char **));
      arg_v[num] = NULL;
      num++;

      if(c == '|') if_pipe = 1;

      if ((strcmp(arg_v[0], "exit") == 0) && (if_back == 0)) {
        if(if_pipe){
          pipe(fd); close(fd[0]); close(fd[1]); pipe(fd);
        } else {
          for (i = 0; i < num; i++) free(arg_v[i]);
          free(arg_v);
          break;
        }
      } else if ((strcmp(arg_v[0], "cd") == 0) && (if_back == 0)) {
        if(if_pipe){
          pipe(fd); close(fd[0]);  close(fd[1]);
        } else {
          if (arg_v[1] == NULL) {
            char home_path[PATH_MAX];
            strcpy(home_path, getenv("HOME"));
            chdir(home_path);
          } else if (chdir(arg_v[1]) == -1) perror("chdir error");
        }
      } else{
        if (if_back) { back_proc++; printf("[%d]   %d\n", back_proc, getpid()); }

        pipe(fd);
        id = fork();
        if (id < 0) {
          perror("fork error");
          exit(4);
        } else if (id == 0) {

          if (if_back == 0) signal(SIGINT, KillProc);

          if (c != '|') { if (redir_outp == 0) dup2(origin[1], 1); }
          else dup2(fd[1], 1);
          close(fd[0]);
          close(fd[1]);

          execvp(arg_v[0], arg_v);
          perror("execvp error");
          exit(1);

        } else {
          if (if_back == 0) { waitpid(id, &status, 0); }
        }
      }

      if (c != '|') if_pipe = 0;

      if(if_pipe == 0) {
        dup2(origin[0], 0);
        dup2(origin[1], 1);
      } else {
        if(redir_outp){
          close(fd[1]); close(fd[0]); pipe(fd);
        }
        else dup2(fd[0], 0);
      }
      close(fd[0]);
      close(fd[1]);
    }


    if(c == '\n') printf("\n >>> ");


    for (i = 0; i < num; i++) free(arg_v[i]);
    free(arg_v);

    c = getchar();
  }

  return 0;
}