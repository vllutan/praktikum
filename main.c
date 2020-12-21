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

int c;

int Shell(char *str_inp, int *origin, int if_return);

int enterCommand(char **arg_v, char*str_inp, int *cur_pos, int *flags, int *origin, int *backup){
  int i=0, redirect=-1, file, num = 0, fd[2];
  char *word;

  if(flags[0]) flags[0] = 1;      //if_pipe, if_pipe
  c = str_inp[*cur_pos];

  while(!( flags[2] || flags[3] || (flags[0] >=2) || flags[6] || flags[4] )) {   // if_and, if_or, if_pipe, if_back, if_div

    word = malloc(sizeof(char *));
    while ((c == ' ') || (c == '\t')) {(*cur_pos)++; c = str_inp[*cur_pos];}

    i = 0;

    if ( c == '<' ) {redirect = 0; (*cur_pos)++; c = str_inp[*cur_pos];}
    else if ( c == '>') {
      redirect = 1; flags[1] = 1;    //redir_outp
      (*cur_pos)++; c = str_inp[*cur_pos];
      if ( c == '>') {
        redirect = 2;
        (*cur_pos)++; c = str_inp[*cur_pos];
      }
    }

    if(flags[0]) flags[0] = 1;      //if_pipe, if_pipe

    if (c == '"') {

      (*cur_pos)++; c = str_inp[*cur_pos];
      while (c != '"') {
        word[i] = c;
        i++;
        word = realloc(word, i * sizeof(char *));
        (*cur_pos)++; c = str_inp[*cur_pos];
      }
      word[i] = '\0';
      if (c == '"') {(*cur_pos)++; c = str_inp[*cur_pos];}
    } else if(c == '(') {
      flags[5] = 1;      //if_braces
      int br_open=0, br_close=0;

      (*cur_pos)++; c = str_inp[*cur_pos];
      while (!( (c == ')') && (br_open == br_close) )){
        if(c == '(') br_open++;
        if(c == ')') br_close++;
        word[i] = c;
        i++;
        word = realloc(word, (i+1) * sizeof(char *));
        (*cur_pos)++; c = str_inp[*cur_pos];
      }
      word[i] = '\n';
      word = realloc(word, (i+1) * sizeof(char *)); i++;
      word[i] = '\0';
      if (c == ')') {(*cur_pos)++; c = str_inp[*cur_pos];}
      else printf(") error: no closing brace\n");

    }else if(c == '`') {
      flags[7] = 1;      //if_quotes

      (*cur_pos)++; c = str_inp[*cur_pos];
      while (c != '`') {
        word[i] = c;
        i++;
        word = realloc(word, (i+1) * sizeof(char *));
        (*cur_pos)++;
        c = str_inp[*cur_pos];
      }
      (*cur_pos)++;
      printf("cur pos %d\n", (*cur_pos));
      word[i] = '\n';
      i++;
      word = realloc(word, (i+1) * sizeof(char *));
      word[i] = '\0';
      printf("word %s\n", word);

      char *new_cmnd = malloc((i + 1) * sizeof(char *));
      strcpy(new_cmnd, word);
      free(word);
      word = malloc(sizeof(char *));
      printf("%s\n", new_cmnd);

      /*pipe(fd);
      //printf("fd closed");
      //Shell(word, origin, 1);
      int n_id = fork();
      printf("fork made\n");
      if (n_id < 0) {
        perror("fork error\n");
        exit(4);
      } else if (n_id == 0) {
      //pipe(fd);
      printf("%d %d\n", fd[0], fd[1]);
      dup2(fd[0], origin[0]);
      //printf("or 0 %d\n", origin[0]);
      dup2(fd[1], origin[1]);
      close(fd[1]);
      close(fd[0]);
      //printf("fd closed");
      int check;
      check = Shell(new_cmnd, origin, 1);
      close(origin[1]);
      dup2(backup[1], origin[1]);
      //perror("return problem: ");
      printf("%d origin[0]\n", check);
      printf("cur pos %d\n", (*cur_pos));
      printf("returned\n");
      // } else if (n_id > 0){
      //close(fd[0]); close(fd[1]);
      //wait(NULL);
      //dup2(backup[1], origin[1]);*/

      int finish_status;
      if((file = open(word, O_RDWR | O_TRUNC | O_CREAT, 0644)) == -1) printf("error file opening\n");
      dup2(file, origin[1]);
      close(file);
      pipe(fd);
      int id = fork();
      if (id < 0) {
        perror("fork error");
        exit(4);
      } else if (id == 0) {


        dup2(fd[1], origin[1]);
        dup2(fd[0], origin[0]);
        close(fd[0]);
        close(fd[1]);

        //for(int j=0;j<8;j++) printf("%d ", flags[j]); printf("\n");

        //printf("braces %s\n", arg_v[0]);
        //printf("%d %d\n", fd[0], fd[1]);
        finish_status = Shell(new_cmnd, origin, 1);
        printf("shell return status %d\n", finish_status);
        if (finish_status > 255) finish_status = finish_status / 255;
        exit(finish_status);


      } else {
        waitpid(id, &finish_status, 0);
        printf("%d %d finish status %d\n", WIFEXITED(finish_status), WEXITSTATUS(finish_status), finish_status);
      }

      printf("ready to read\n");
      int ch;
      i = 0;
      while (read(origin[0], &ch, sizeof(char)) > 0) {
        printf("%c", ch);
        word[i] = ch;
        i++;
        word = realloc(word, (i + 1) * sizeof(char *));
      }

      close(fd[0]); close(fd[1]);
      dup2(backup[0], origin[0]); dup2(backup[1], origin[1]);
      printf("closed\n");

      word[i] = '\0';

    } else {

      while ((c != ' ') && (c != '\t') && (c != '\n') && (c != EOF) && (c != ';') && (c != '>') && (c != '<') && (c != '\0')) {
        if (flags[6] == 1) {    //if_back
          if(c == '&'){
            flags[2] = 1;      //if_and
            flags[6] = 0;    //if_back
            break;
          } else {
            flags[6] = 2;    //if_back
            (*cur_pos)--; c = str_inp[*cur_pos];
            break;
          }
        } else if (c == '&') {
          flags[6] = 1;    //if_back
          (*cur_pos)++; c = str_inp[*cur_pos];
        }  else if (flags[0] >= 2) {     //if_pipe
          if(c == '|') {
            flags[0] = flags[0] - 2;     //if_pipe, if_pipe
            flags[3] = 1;     //if_or
            break;
          } else {
            (*cur_pos)--; c = str_inp[*cur_pos];
            break;
          }
        } else if(c == '|') {
          flags[0] = flags[0] + 2;      //if_pipe, if_pipe
          (*cur_pos)++; c = str_inp[*cur_pos];
        } else {
          word[i] = c;
          i++;
          word = realloc(word, (i+1) * sizeof(char *));
          (*cur_pos)++; c = str_inp[*cur_pos];
        }
      }
      word[i] = '\0';
    }

    if((flags[0] >= 2) && (c != '|')) {(*cur_pos)--; c = str_inp[*cur_pos];}


    while ((c == ' ') || (c == '\t')) {(*cur_pos)++; c = str_inp[*cur_pos];}

    if( (c == '\n') || (c == EOF) || (c == ';') || (c == '\0')) flags[4] = 1;    //if_div

    if(flags[6] && (c != '\n')) {    //if_back
      flags[6] = 3;           //if_back
      fprintf(stderr, "& error: not at the end\n");
    }


    if(flags[6] < 2) {    //if_back
      switch (redirect) {
        case 0:
          if ((file = open(word, O_RDONLY, 0644)) == -1) printf("file open error\n");
          dup2(file, origin[0]);
          close(file);
          redirect = -1;
          break;
        case 1:
          if((file = open(word, O_RDWR | O_TRUNC | O_CREAT, 0644)) == -1) printf("error file opening\n");
          dup2(file, origin[1]);
          close(file);
          redirect = -1;
          break;
        case 2:
          file = open(word, O_RDWR | O_APPEND, 0644);
          dup2(file, origin[1]);
          close(file);
          redirect = -1;
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
  return num;
}

void KillProc(int sig){
  exit(0);
}


int main(){
  char *str_inp;
  str_inp = malloc(sizeof(char*));

  int origin[2];
  origin[0] = 0;
  origin[1] = 1;

  printf(" >>> ");

  Shell(str_inp, origin, 0);

  return 0;
}

int Shell(char *str_inp, int *origin, int if_return) {

  int num = 0;

  int backup[2];
  pipe(backup);
  dup2(origin[0], backup[0]);
  dup2(origin[1], backup[1]);

  int fd[2];

  int i=0, status, back_proc=0, back_released=1, finish_status=0, if_do=1;
  int str_size = 0;
  char *new_str;
  int cur_pos = 0;
  pid_t id;
  char **arg_v;

  int flags[8]={0,0,0,0,0,0,0,0};

  while ( 1 ) {
    arg_v = malloc(sizeof(char **));
    arg_v[0] = NULL;
    for (int j = 1; j < 8; j++) flags[j] = 0;

    signal(SIGINT, SIG_IGN);

    if (cur_pos == strlen(str_inp)) {
      //----------------------------- INPUT NEW LINE FROM CMD -------------------

      if(if_return) break;

      str_size = 0;
      new_str = malloc(10 * sizeof(char *));
      free(str_inp);
      str_inp = malloc(sizeof(char *));
      do {
        fgets(new_str, 10, stdin);
        str_inp = realloc(str_inp, (str_size + strlen(new_str)) * sizeof(char *));
        memcpy(str_inp + str_size, new_str, strlen(new_str) + 1);
        str_size = str_size + strlen(new_str);
      } while ((str_inp[str_size] != '\0') || (str_inp[str_size - 1] != '\n'));
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

    num = enterCommand(arg_v, str_inp, &cur_pos, flags, origin, backup);

//   --------------- PROCESSING COMMAND in IF cycle ---------------------------------------

    if (arg_v[0] && (flags[6] < 2)) {          //if_back
      arg_v = realloc(arg_v, (num + 1) * sizeof(char **));
      arg_v[num] = NULL;
      num++;

      if (if_do == 0) {
        if (((finish_status == 0) && flags[3]) || ((finish_status != 0) && flags[2])) {   //if_or, if_and
          if_do = 0;
          flags[2] = 0;  //if_and
          flags[3] = 0;    //if_or
        } else if_do = 1;
      } else {
        if ((strcmp(arg_v[0], "exit") == 0) && (flags[6] == 0)) {   //if_back
          if (flags[0]) {      //if_pipe
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
        } else if ((strcmp(arg_v[0], "cd") == 0) && (flags[6] == 0)) {       //if_back
          if (flags[0]) {            //if_pipe
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

            if (flags[6] == 0) signal(SIGINT, KillProc);         //if_back

            if (flags[4] || flags[2] || flags[3]) {       //if_div, if_and, if_or
              if (flags[1] == 0) dup2(backup[1], origin[1]);     //redir_outp
            } else dup2(fd[1], origin[1]);
            close(fd[0]);
            close(fd[1]);


            if (flags[5]) {     //if_braces
              finish_status = Shell(arg_v[0], origin, 1);
              if(finish_status > 255) finish_status = finish_status / 255;
              exit(finish_status);

            } else {
              execvp(arg_v[0], arg_v);
              perror("execvp error");
              exit(1);
            }

          } else {
            if (flags[6]) {       //if_back
              back_proc++;
              printf("[%d]   %d\n", back_proc, id);
            } else {
              waitpid(id, &finish_status, 0);
              if (((finish_status == 0) && flags[3]) || ((finish_status != 0) && flags[2])) {      //if_or, if_and
                if_do = 0;
              }
            }
          }
        }

        if (flags[4] || flags[2] || flags[3]) flags[0] = 0;      //if_div, if_and, if_or, if_pipe

        if (flags[0] == 0) {           //if_pipe
          dup2(backup[0], origin[0]);
          dup2(backup[1], origin[1]);
        } else {
          if (flags[1]) {           //redir_outp
            close(fd[1]);
            close(fd[0]);
            pipe(fd);
          } else dup2(fd[0], origin[0]);

        }
        close(fd[0]);
        close(fd[1]);
      }
    }
    if ((c == '\n') && !if_return) printf("\n >>> ");

    for (i = 0; i < num; i++) free(arg_v[i]);
    free(arg_v);

    cur_pos++; //c = str_inp[cur_pos];

  }

  close(backup[0]); close(backup[1]);

  return finish_status;
}