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

//char *word;
//char **arg_v;
//char *str_inp;
//int cur_pos;
int c, num=0, if_back=0, if_pipe=0, redir_inp=0, redir_outp=0, if_and=0, if_or=0, if_div=0;

void enterCommand(char **arg_v, char*str_inp, int *cur_pos){
  int i=0, redirect=-1, file;
  char *word;

  if(if_pipe) if_pipe = 1;
  c = str_inp[*cur_pos];
  while(!( if_and || if_or || (if_pipe >=2) || if_back || if_div )) {

    word = malloc(sizeof(char *));
    while ((c == ' ') || (c == '\t')) {(*cur_pos)++; c = str_inp[*cur_pos];}

    i = 0;

    if ( c == '<' ) {redirect = 0; redir_inp = 1; (*cur_pos)++; c = str_inp[*cur_pos];}
    else if ( c == '>') {
      redirect = 1; redir_outp = 1;
      (*cur_pos)++; c = str_inp[*cur_pos];
      if ( c == '>') {
        redirect = 2;
        (*cur_pos)++; c = str_inp[*cur_pos];
      }
    }

    if(if_pipe) if_pipe = 1;

    if (c != '"') {

      while ((c != ' ') && (c != '\t') && (c != '\n') && (c != EOF) && (c != ';') && (c != '>') && (c != '<')) {
        if (if_back == 1) {
          if(c == '&'){
            if_and = 1;
            if_back = 0;
            break;
          } else {
            if_back = 2;
            (*cur_pos)--; c = str_inp[*cur_pos];
            break;
          }
        } else if (c == '&') {
          if_back = 1;
          (*cur_pos)++; c = str_inp[*cur_pos];
        }  else if (if_pipe>=2) {
          if(c == '|') {
            if_pipe = if_pipe - 2;
            if_or = 1;
            break;
          } else {
            (*cur_pos)--; c = str_inp[*cur_pos];
            break;
          }
        } else if(c == '|') {
          if_pipe = if_pipe + 2;
          (*cur_pos)++; c = str_inp[*cur_pos];
        } else {
          word[i] = c;
          i++;
          word = realloc(word, i * sizeof(char *));
          (*cur_pos)++; c = str_inp[*cur_pos];
        }
      }
      word[i] = '\0';
    } else {

      (*cur_pos)++; c = str_inp[*cur_pos];
      while (c != '"') {
        word[i] = c;
        i++;
        word = realloc(word, i * sizeof(char *));
        (*cur_pos)++; c = str_inp[*cur_pos];
      }
      word[i] = '\0';
      if (c == '"') {(*cur_pos)++; c = str_inp[*cur_pos];}
    }

    while ((c == ' ') || (c == '\t')) {(*cur_pos)++; c = str_inp[*cur_pos];}

    if( (c == '\n') || (c == EOF) || (c == ';') ) if_div = 1;

    if(if_back && (c != '\n')) {
      if_back = 3;
      fprintf(stderr, "& error: not at the end\n");
    }

    //printf("%d %d %d %d %d %d %d\n", if_back, if_pipe, redir_inp, redir_outp, if_and, if_or, if_div);

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
  }
}

void KillProc(int sig){
  exit(0);
}

int Shell();

int main(){
  char *str_inp;
  str_inp = malloc(sizeof(char*));

  printf(" >>> ");

  Shell(str_inp);

  return 0;
}

int Shell(char *str_inp) {

  int i=0, fd[2], origin[2], status, back_proc=0, back_released=1, finish_status=0, if_do=1;
  int str_size = 0;
  char *new_str;
  int cur_pos = 0;
  pid_t id;

  pipe(origin);
  dup2(0, origin[0]);
  dup2(1, origin[1]);

  char **arg_v;

  cur_pos = 0;

  while ( 1 ) {
    arg_v = malloc(sizeof(char **));
    arg_v[0] = NULL;
    num = 0;
    if_back = 0;
    if_div = 0;
    redir_outp = 0;
    redir_inp = 0;
    if_and = 0;
    if_or = 0;

    signal(SIGINT, SIG_IGN);

    if (cur_pos == strlen(str_inp)) {
      //----------------------------- INPUT NEW LINE FROM CMD -------------------

      str_size = 0;
      new_str = malloc(10 * sizeof(char *));
      free(str_inp);
      str_inp = malloc(sizeof(char*));
      do {
        fgets(new_str, 10, stdin);
        str_inp = realloc(str_inp, (str_size + strlen(new_str))*sizeof(char *));
        memcpy(str_inp + str_size, new_str, strlen(new_str) + 1);
        str_size = str_size + strlen(new_str);
      } while((str_inp[str_size] != '\0') || (str_inp[str_size-1] != '\n'));
      free(new_str);
      cur_pos = 0;
    }

//  -------------------- PROCESSING BACK PROCESSES --------------------------------------------

    while ((id = waitpid(-1, &status, WNOHANG)) > 0) {
      if (WIFEXITED(status)) {
        printf("[%d] finished well with status %d\n\n", back_released, WEXITSTATUS(status));
        back_released++;
        back_proc--;
      } else if (WIFSIGNALED(status)) {
        printf("[%d] finished, killed by signal %d\n\n", back_released, WTERMSIG(status));
        back_released++;
        back_proc--;
      } else if (WIFSTOPPED(status)) {
        printf("[%d] finished, stopped by signal %d\n\n", back_released, WSTOPSIG(status));
        back_released++;
        back_proc--;
      }
      if (back_proc == 0) back_released = 1;
    }

    //--------END


    enterCommand(arg_v, str_inp, &cur_pos);

//   --------------- PROCESSING COMMAND in IF cycle ---------------------------------------

    if (arg_v[0] && (if_back < 2)) {
      arg_v = realloc(arg_v, (num + 1) * sizeof(char **));
      arg_v[num] = NULL;
      num++;

      if (if_do == 0) {
        if (((finish_status == 0) && if_or) || ((finish_status != 0) && if_and)) {
          if_do = 0;
          if_and = 0;
          if_or = 0;
        } else if_do = 1;
      } else {
        if ((strcmp(arg_v[0], "exit") == 0) && (if_back == 0)) {
          if (if_pipe) {
            pipe(fd);
            close(fd[0]);
            close(fd[1]);
            pipe(fd);
          } else {
            for (i = 0; i < num; i++) free(arg_v[i]);
            free(arg_v);
            free(str_inp);
            break;
          }
        } else if ((strcmp(arg_v[0], "cd") == 0) && (if_back == 0)) {
          if (if_pipe) {
            pipe(fd);
            close(fd[0]);
            close(fd[1]);
            pipe(fd);
          } else {
            if (arg_v[1] == NULL) {
              char home_path[PATH_MAX];
              strcpy(home_path, getenv("HOME"));
              chdir(home_path);
            } else if (chdir(arg_v[1]) == -1) perror("chdir error");
          }
        } else {


          pipe(fd);
          id = fork();
          if (id < 0) {
            perror("fork error");
            exit(4);
          } else if (id == 0) {


            if (if_back == 0) signal(SIGINT, KillProc);

            if (if_div || if_and || if_or) { if (redir_outp == 0) dup2(origin[1], 1); }
            else dup2(fd[1], 1);
            close(fd[0]);
            close(fd[1]);

            execvp(arg_v[0], arg_v);
            perror("execvp error");
            exit(1);

          } else {
            if (if_back) {
              back_proc++;
              printf("[%d]   %d\n", back_proc, id);
            } else {
              waitpid(id, &status, 0);
              finish_status = status;
              //printf("%d\n", finish_status);
              if (((finish_status == 0) && if_or) || ((finish_status != 0) && if_and)) {
                if_do = 0;
              }
            }
          }
        }

        if (if_div || if_and || if_or) if_pipe = 0;

        if (if_pipe == 0) {
          dup2(origin[0], 0);
          dup2(origin[1], 1);
        } else {
          if (redir_outp) {
            close(fd[1]);
            close(fd[0]);
            pipe(fd);
          } else dup2(fd[0], 0);
        }
        close(fd[0]);
        close(fd[1]);
      }
    }
      if (c == '\n') printf("\n >>> ");

      for (i = 0; i < num; i++) free(arg_v[i]);
      free(arg_v);                  ///////////////////////// !!!!!!!!!!!!!!!!!!!!!!!!!

      //if (!(if_pipe || if_back)) {cur_pos++; c = str_inp[cur_pos];} //c = getchar();
      cur_pos++; c = str_inp[cur_pos];


    //printf("cur %d, len %ld, char %c\n", cur_pos, strlen(str_inp), c);

  }

  return 0;
}