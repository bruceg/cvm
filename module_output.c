#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "module.h"

char outbuffer[BUFSIZE];
unsigned outbuflen;
static char* outbufptr;

static int fact(int number, const char* data, unsigned len)
{
  /* Always leave room for a trailing NUL */
  if (outbuflen + len + 3 > BUFSIZE) {
    outbuflen = BUFSIZE;
    return 0;
  }
  outbuflen += len + 2;
  *outbufptr++ = (char)number;
  memcpy(outbufptr, data, len);
  outbufptr += len;
  *outbufptr++ = 0;
  return 1;
}

void fact_start(void)
{
  outbuflen = 1;
  outbufptr = outbuffer + outbuflen;
}

int fact_str(int number, const char* data)
{
  if (!data) return 0;
  return fact(number, data, strlen(data));
}

void fact_end(int code)
{
  if (outbuflen >= BUFSIZE) code = CVME_BAD_MODDATA;
  if (code) {
    outbuffer[0] = code;
    outbuflen = 1;
  }
  else {
    outbuffer[0] = 0;
    *outbufptr++ = 0;
    ++outbuflen;
  }
}

int fact_uint(int number, unsigned data)
{
  char buf[64];
  char* ptr;
  
  if (!data)
    return fact(number, "0", 1);
  ptr = buf + 63;
  *ptr-- = 0;
  while (data) {
    *ptr-- = (data % 10) + '0';
    data /= 10;
  }
  ++ptr;
  return fact(number, ptr, buf+63-ptr);
}
