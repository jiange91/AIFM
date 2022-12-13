#include <stdio.h>

static FILE *in = NULL;
static char instring[201];

int init_file(const char *file) {
  if(( in = fopen( file, "r")) == NULL )
    return 0;
  return 1;
}

int read_l_l(long *t, long *h) {
  fgets( instring, 200, in );
  if( sscanf( instring, "%ld %ld\n", t, h ) != 2 )
    return 0;
  return 1;
}

int read_l_l_l(long *t, long *h, long *c) {
  fgets( instring, 200, in );
  if( sscanf( instring, "%ld %ld %ld\n", t, h, c ) != 3 )
    return 0;
  return 1;
}

void finish_reading() {
  fclose(in);
}