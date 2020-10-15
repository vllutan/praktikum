#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

int ch;

struct node{
  char *word;
  int amount;
  struct node *left;
  struct node *right;
};

int isword(int c){
  return ( isalnum(c) || c=='-' || c=='_' ) ? 1 : 0;
}

struct node *AddInFreqTree(struct node *head, char *str, int num, FILE *f_out){
  struct node *p, *q;
  p = head; q = NULL;
  
  while (p != NULL) {
    q = p;
    if (num > p->amount) p = p->right;
    else p = p->left;
  }

  struct node *new;
  new = (struct node*)malloc(sizeof(struct node));
  new->word = malloc(strlen(str) + 1);
  strcpy(new->word, str);
  new->amount = num;
  new->left = NULL;
  new->right = NULL;

  if (q == NULL) head = new;
  else if (num > q->amount) q->right = new;
  else q->left = new;

  return head;
}

struct node *AddInDict(struct node *head, char *str){
  struct node *p, *q;
  p = head; q = NULL;
  int res;

  while (p != NULL) {
    q = p;
    res = strcmp(str, p->word);
    if (res == 0) {
      p->amount++;
      return head;
    } else if (res > 0) p = p->right;
    else p = p->left;
  }

  struct node *new;
  new = (struct node*)malloc(sizeof(struct node));
  new->word = malloc(strlen(str) + 1);
  strcpy(new->word, str);
  new->amount = 1;
  new->left = NULL;
  new->right = NULL;

  if (q == NULL) head = new;
  else if (strcmp(str, q->word) > 0) q->right = new;
  else q->left = new;
  
  return head;
}

void PrintTree(struct node *head, FILE *f_out, int total){
  if (head != NULL){
    PrintTree(head->right, f_out, total);
    double freq = (head->amount)/(total * 1.0);
    fprintf(f_out, "%s %d %f\n", head->word, head->amount, freq);
    PrintTree(head->left, f_out, total);
  }
}

struct node *MakeTreeFromDict(struct node *d_head, struct node *f_tree_head, FILE *f_out){
  if (d_head != NULL){
    f_tree_head = MakeTreeFromDict(d_head->left, f_tree_head, f_out);
    f_tree_head = AddInFreqTree(f_tree_head, d_head->word, d_head->amount, f_out);
    f_tree_head = MakeTreeFromDict(d_head->right, f_tree_head, f_out);
  }
  return f_tree_head;
}

void DeleteTree(struct node *head){
  if (head != NULL) {
    DeleteTree(head->left);
    DeleteTree(head->right);
    free(head->word);
    free(head);
  }
}

char *NewWord(FILE *f_inp){
  char *str = NULL;
  int i = 0, str_size = 0;
  
  while( isspace(ch) ) ch = getc(f_inp);

  if ( isword(ch) ) {
    while ( isword(ch) ) {
      if (i == str_size) {
        str_size = 2 * str_size + 1;
        str = (char *)realloc(str, str_size);
        if (str == NULL) printf("str realloc error\n");
      }
      str[i] = ch;
      i++;
      ch = getc(f_inp);
    }
    str = realloc(str, str_size + 1);
    str[i] = '\0';
  } else if (ch != EOF) {
    str = malloc(2 * sizeof(int));
    str[0] = ch;
    str[1] = '\0';
    ch = getc(f_inp);
  }

  return str;
}

int main(int argc, char *argv[]) {
  FILE *f_inp = stdin;
  FILE *f_out = stdout;

  if (argc >= 2) {
    if ( !strcmp(argv[1], "-i") ) f_inp = fopen(argv[2], "r");
    else if ( !strcmp(argv[1], "-o") ) f_out = fopen(argv[2], "w");
    if (argc >= 4) {
      if ( !strcmp(argv[3], "-i") ) f_inp = fopen(argv[4], "r");
      else if ( !strcmp(argv[3], "-o") ) f_out = fopen(argv[4], "w");
    }
  }

  if (f_inp == NULL) fprintf(stderr, "f_inp opened incorrectly");
  if (f_out == NULL) fprintf(stderr, "f_out opened incorrectly");

  struct node *dict_head = NULL;
  char *str = NULL;
  int total = 0;

  ch = getc(f_inp);
  while (ch != EOF) {
    str = NewWord(f_inp);
    if (str != NULL) {
      total++;
      dict_head = AddInDict(dict_head, str);
      free(str);
    }
  }

  struct node *freq_tree_head = NULL;

  freq_tree_head = MakeTreeFromDict(dict_head, freq_tree_head, f_out);

  PrintTree(freq_tree_head, f_out, total);

  DeleteTree(dict_head);
  DeleteTree(freq_tree_head);

  if (f_inp != stdin) fclose(f_inp);
  if (f_out != stdout) fclose(f_out);

  return 0;
}
