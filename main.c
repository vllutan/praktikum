#include <stdio.h>

int if_be = 0;

unsigned short get_Symbol(FILE *f_inp){
  unsigned short symb=0, byte=0;

  if (fread(&symb, 1, 1, f_inp) ) {
    if (fread(&byte, 1, 1, f_inp)) {
      byte = byte << 8;
      symb = symb | byte;
    } else {
      fprintf(stderr, "Odd number of bytes\n");
      return get_Symbol(f_inp);
    }

    if (if_be) symb = (0xff00 & (symb << 8)) | ((symb >> 8) & 0xff);
  }
  return symb;
}

void Recode(FILE *f_out, unsigned short symbol){
  unsigned char ch[3];
  
  if (symbol < 128) {
    ch[0] = symbol & 0x7f;
    fwrite(ch, 1, 1, f_out);
  }
  else if(symbol < 2048) {
    ch[0] = ((symbol >> 6) & 0x1f) | 0xc0;
    ch[1] = (symbol & 0x3f) | 0x80;
    fwrite(ch, 1, 2, f_out);
  }
  else {
    ch[0] = ((symbol >> 12) & 0xf) | 0xe0;
    ch[1] = ((symbol >> 6) & 0x3f) | 0x80;
    ch[2] = ((symbol & 0x3f)) | 0x80;
    fwrite(ch, 1, 3, f_out);
  }
}

int main(int argc, char **argv) {
  FILE *f_inp = stdin;
  FILE *f_out = stdout;

  if (argc > 1) {
    f_inp = fopen(argv[1], "rb");
    if (argc > 2) f_out = fopen(argv[2], "wb");
    if (argc > 3) fprintf(stderr, "More parameters than needed\n");
  }

  if(f_inp == NULL) { fprintf(stderr, "Incorrect input file opening\n"); return -2; }
  if(f_out == NULL) { fprintf(stderr, "Incorrect output file opening\n"); return -2; }

  unsigned short bom, sym;

  fread(&bom, 2, 1, f_inp);

  if ( (bom & 0xff) == 0xfe){
    if ( (bom & 0xff00) == 0xff00 ) {
      if_be = 1;
      sym = get_Symbol(f_inp);
    } else {
      fread(&sym, 1, 1, f_inp);
      sym = sym << 8;
      sym = sym | (bom >> 8);
    }
    bom = 0xfeff;
  } else if ( (bom & 0xff) == 0xff){
    if ( (bom & 0xff00) != 0xfe00 ) {
      if_be = 1;
      fread(&sym, 1, 1, f_inp);
      sym = sym | (bom & 0xff00);
      bom = 0xfeff;
    } else sym = get_Symbol(f_inp);
  }
  if (bom != 0xfeff) {
    fprintf(stderr, "Warning: no BOM found, LE is used\n");
    sym = bom;
  } else Recode(f_out, bom);

  while( !feof(f_inp) ) {
    Recode(f_out, sym);
    sym = get_Symbol(f_inp);
  }

  fclose(f_inp);
  fclose(f_out);

  return 0;
}
