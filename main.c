#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>

int main() {
  char *word;
  char **arg_v;
  arg_v = malloc(sizeof(char**));
  int c;
  int i=0, num=0;

  c = getchar();
  while ( (c != '\n') && (c != EOF) ){

    word = malloc(sizeof(char*));
    while((c == ' ') || (c == '\t')) c = getchar();

    if (c == '\n') break;
    i=0;

    if (c != '"') {

       while ((c != ' ') && (c != '\t') && (c != '\n')) {
         word[i] = c;
         i++;
         word = realloc(word, i* sizeof(char*));
         c = getchar();
       }
       word[i] = '\0';
    } else {

      while ((c = getchar()) != '"') {
        word[i] = c;
        i++;
        word = realloc(word, i* sizeof(char*));
      }
      word[i] = '\0';
      c = getchar();
    }

    arg_v = realloc(arg_v, (num+1)* sizeof(char**));
    arg_v[num] = malloc((strlen(word) + 1)* sizeof(char*));
    strcpy(arg_v[num], word);
    num++;
    free(word);
  }

  if (arg_v[0]) {
    if (strcmp(arg_v[0], "exit") == 0) {
      if (arg_v[1]) printf("%s\n", arg_v[1]);
      //break;                                         will appear when a cycle for several commands will be added
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

        pid_t id = fork();
        if (id < 0) {
          perror("fork error");
        } else if (id == 0) {
          execvp(arg_v[0], arg_v);
          perror("execvp error");
          exit(1);
        } else {
          wait(NULL);
        }
      }
    }
  }
  for(i=0; i<num; i++) free(arg_v[i]);
  free(arg_v);

  return 0;
}