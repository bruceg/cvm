#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "module.h"

#define BUFSIZE 4096
static char buffer[BUFSIZE];
static char* bufptr = buffer;
static char* bufend = buffer + BUFSIZE;

static void fact(int number, const char* data, unsigned len)
{
  /* Always leave room for a trailing NUL */
  if (bufptr + len + 3 >= bufend) exit(111);
  *bufptr++ = (char)number;
  memcpy(bufptr, data, len);
  bufptr += len;
  *bufptr++ = 0;
}

void fact_str(int number, const char* data)
{
  if (!data) exit(111);
  fact(number, data, strlen(data));
}

void fact_end(void)
{
  char* ptr;
  size_t wr;
  
  *bufptr++ = 0;
  bufend = bufptr;
  for (ptr = buffer; ptr < bufend; ptr += wr) {
    wr = write(1, ptr, bufend - ptr);
    if (wr == -1 || wr == 0) exit(111);
  }
  exit(0);
}

void fact_uint(int number, unsigned data)
{
  char buf[64];
  char* ptr;
  
  if (!data)
    fact(number, "0", 1);
  else {
    ptr = buf + 63;
    *ptr-- = 0;
    while (data) {
      *ptr-- = (data % 10) + '0';
      data /= 10;
    }
    ++ptr;
    fact(number, ptr, buf+63-ptr);
  }
}
