#include "error.h"

extern int linenum;
extern int errorCount;

void printError(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  printf("##########Error at Line %d: ", linenum);
  vprintf(fmt, ap);
  printf("##########\n");
  va_end(ap);
  errorCount += 1;
}
