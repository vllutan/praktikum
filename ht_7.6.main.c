#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  FILE *f1 = fopen(argv[2], "r");
  FILE *f2 = fopen(argv[3], "r");

  if(f1==NULL) fprintf(stderr, "error f1");
  if(f2==NULL) fprintf(stderr, "error f2");

  char *s1 = NULL;
  int c1, c2, i;
  int if_equal = 1;

  while (((c1 = getc(f1)) != EOF) ){
    c2 = getc(f2);
    int length_s1 = 0, max_size_s1 = 0;

    while ((c1 != '\n') && (c1 != EOF)){
      if (length_s1 == max_size_s1) {
        max_size_s1 = 2 * max_size_s1 + 1;
        s1 = realloc(s1, max_size_s1);
      }
      s1[length_s1] = c1;
      length_s1++;
      c1 = getc(f1);
    }

    i = 0;
    while( (c2 != '\n') && (c2 != EOF)) {
      printf("%d %c ", i, c2);
      if (c2 != s1[i]) if_equal = 0;
      c2 = getc(f2);
      i++;
    }

    if (i < length_s1) if_equal = 0;

    if (if_equal==1) putchar('1');
    else if (if_equal==0) putchar('0');
    for(int i=0; i<length_s1; i++) putchar(s1[i]);
    putchar('\n');

    if (!if_equal) {
      printf("First line with mismatch: ");
      for(int i=0; i<length_s1; i++) putchar(s1[i]);
      putchar('\n');
      break;
    } else {
      s1 = malloc(0);
      if_equal = 1;
    }
  }

  c2 = getc(f2);
  if ((c2 == EOF) && if_equal) printf("Texts are equal");
  else if (c2 != EOF) printf("Mismatch after the end of the 1st file (1st - EOF, 2nd - continues)");

  free(s1);


  return 0;
}