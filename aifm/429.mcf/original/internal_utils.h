#include <stdint.h>


extern int init_file(const char *file);
extern int read_l_l(long *t, long *h);
extern int read_l_l_l(long *t, long *h, long *c);
extern void finish_reading();