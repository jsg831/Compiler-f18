#include "error.h"

extern int linenum;
extern int errorCount;

void printError(const char *fmt, ...) {
  char msg[256] = "", buf[256] = "";
  va_list ap;
  va_start(ap, fmt);
  sprintf(buf, "Error at Line %d: ", linenum);
  strcat(msg, buf);
  vsprintf(buf, fmt, ap);
  strcat(msg, buf);
  printf("##########%-80s##########\n", msg);
  va_end(ap);
  errorCount += 1;
}
