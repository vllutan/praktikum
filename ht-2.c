#include <stdio.h>
#include <stdlib.h>

char *max_string=NULL;

int main(){
  int ch, numb_max=0, i=0;

  while((ch=getchar()) != EOF) {
    int size_str = 0, numb_str = 0;
    char *inp_string=NULL;

    while (ch != '\n') {
      if (numb_str == size_str) {
        size_str = 2 * size_str + 1;
        inp_string = realloc(inp_string, size_str);
      }
      inp_string[numb_str] = ch;
      numb_str++;
      ch = getchar();
    }

    printf("\n%d ", numb_str);
    for (i = 0; i < numb_str; i++) putchar(inp_string[i]);
    putchar('\n');

//    printf("%d %d\n", numb_max, numb_str);
    if (numb_max < numb_str) {
      max_string = inp_string;
      numb_max = numb_str;
//      printf("%d %d\n", numb_max, numb_str);
    } else free(inp_string);

    printf("%d ", numb_max);
    for (i = 0; i < numb_max; i++) putchar(max_string[i]);
    putchar('\n');

  }

  printf("\n%d ", numb_max);
  for(i=0; i<numb_max; i++) putchar(max_string[i]);
  putchar('\n');

  free(max_string);

  return 0;
}