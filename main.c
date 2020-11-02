#include <stdio.h>

unsigned char bytes[3];
unsigned char bytes16[2]; //0 - higher, 1 - lower
int if_be=0;

void Make_UTF16(int num, FILE *f_out, FILE *f_inp){
  if(num == 1){
    bytes16[0] = 0;
    bytes16[1] = bytes[0];
  } else if (num == 2){
    bytes16[0] = bytes[0] >> 2;
    bytes16[1] = (bytes[0] << 6) | bytes[1];
  } else if (num == 3){
    bytes16[0] = (bytes[0] << 4) | (bytes[1] >> 2);
    bytes16[1] = (bytes[1] << 6) | bytes[2];
  }

  if (if_be) {
    fwrite(bytes16, 1, 2, f_out);
  } else {
    fwrite(bytes16+1, 1, 1, f_out);
    fwrite(bytes16, 1, 1, f_out);
  }

  if (num == 1){
    bytes[0] = bytes[1];
    bytes[1] = bytes[2];
    for (int i=2; i<=2;i++) if( fread(bytes+i, 1, 1, f_inp) == 0) {bytes[i] = 0xff;}
  } else if (num == 2){
    bytes[0] = bytes[2];
    for (int i=1; i<=2;i++) if( fread(bytes+i, 1, 1, f_inp) == 0) {bytes[i] = 0xff;}
  } else if (num == 3) {
    for (int i = 0; i <= 2; i++)
      if (fread(bytes + i, 1, 1, f_inp) == 0) bytes[i] = 0xff;
  }
}

int get_Symbol(FILE *f_out, FILE *f_inp){
  if ( (bytes[0] & 0x80) == 0) {
    Make_UTF16(1, f_out, f_inp);
    return 1;
  } else if ( (bytes[0] & 0xe0) == 0xc0){
    if ( (bytes[1] & 0xc0) == 0x80) {
      bytes[0] = bytes[0] & 0x1f;
      bytes[1] = bytes[1] & 0x3f;
      Make_UTF16(2, f_out, f_inp);
      return 2;
    } else {
      fprintf(stderr, "Incorrect byte sequence: no continuing byte (for 2-byte symbols); "
                      "Position: %ld; Symbol: %x\n", ftell(f_inp)-2, bytes[1]);
      bytes[0] = bytes[2];
      for (int i=1; i<=2;i++) if( fread(bytes+i, 1, 1, f_inp) == 0) {bytes[i] = 0xff;}
    }
  } else if ( (bytes[0] & 0xf0) == 0xe0) {
    if ( (bytes[1] & 0xc0) == 0x80){
      if ( (bytes[2] & 0xc0) == 0x80) {
        bytes[0] = bytes[0] & 0xf;
        bytes[1] = bytes[1] & 0x3f;
        bytes[2] = bytes[2] & 0x3f;
        Make_UTF16(3, f_out, f_inp);
        return 3;
      } else {
        fprintf(stderr, "Incorrect byte sequence: no 2nd continuing byte (for 3-byte symbols); "
                        "Position: %ld; Symbol: %x\n", ftell(f_inp)-1, bytes[2]);
        for (int i=0; i<=2;i++) if( fread(bytes+i, 1, 1, f_inp) == 0) {bytes[i] = 0xff;}
      }
    } else {
      fprintf(stderr, "Incorrect byte sequence: no 1st continuing byte (for 3-byte symbols); "
                      "Position: %ld; Symbol: %x\n", ftell(f_inp)-2, bytes[1]);
      bytes[0] = bytes[2];
      for (int i=1; i<=2;i++) if( fread(bytes+i, 1, 1, f_inp) == 0) {bytes[i] = 0xff;}
    }
  } else {
    fprintf(stderr, "Incorrect byte sequence: doesn't start with anything appropriate; "
                    "Position: %ld; Symbol: %x\n", ftell(f_inp)-3, bytes[0]);
    bytes[0] = bytes[1];
    bytes[1] = bytes[2];
    for (int i=2; i<=2;i++) if( fread(bytes+i, 1, 1, f_inp) == 0) {bytes[i] = 0xff;}
  }
}

int main(int argc, char **argv){
  FILE *f_inp = stdin;
  FILE *f_out = stdout;

  if (argc > 1) {
    f_inp = fopen(argv[1], "rb");
    if (argc > 2) f_out = fopen(argv[2], "wb");
    if (argc > 3) fprintf(stderr, "More parameters than needed\n");
  }

  if(f_inp == NULL) { fprintf(stderr, "Incorrect input file opening\n"); return -2; }
  if(f_out == NULL) { fprintf(stderr, "Incorrect output file opening\n"); return -2; }

  for (int i=0; i<=2;i++) if( fread(bytes+i, 1, 1, f_inp) == 0) {bytes[i] = 0xff;}

  if (bytes[0] == 0xef) {
    if (bytes[1] == 0xbb){
      if(bytes[2] == 0xbf){
        if_be = 1;
        Make_UTF16(3, f_out, f_inp);
      } else {
        bytes[0] = bytes[2];
        for (int i=1; i<=2;i++) if( fread(bytes+i, 1, 1, f_inp) == 0) {bytes[i] = 0xff;}
      }
    } else {
      bytes[0] = bytes[1];
      bytes[1] = bytes[2];
      for (int i=2; i<=2;i++) if( fread(bytes+i, 1, 1, f_inp) == 0) {bytes[i] = 0xff;}
    }
  } else {
    putc(0xff, f_out);
    putc(0xfe, f_out);
    fprintf(stderr, "Warning: no BOM found, LE is used\n");
  }

  while( bytes[0] != 0xff ) {
    get_Symbol(f_out, f_inp);
  }

  fclose(f_inp);
  fclose(f_out);

  return 0;
}