#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "module.h"

#define BUFSIZE 4096
static char buffer[BUFSIZE + 1];
static const char* bufend;
const char* service_name;
const char* account_name;
const char* credentials;
static const char* credptr;

static const char* strnul(const char* ptr)
{
  if (!ptr) return 0;
  ptr = strchr(ptr, 0);
  return ptr ? ptr + 1 : 0;
}

void read_input(void)
{
  size_t pos;
  size_t rd;

  pos = 0;
  for (;;) {
    rd = read(0, buffer+pos, BUFSIZE-pos);
    if (rd == -1) exit(111);
    if (rd == 0) break;
    pos += rd;
    if (pos >= BUFSIZE) break;
  }
  bufend = buffer + pos;
  buffer[pos] = 0;

  account_name = buffer;
  credentials = strnul(account_name);
  credptr = credentials;

  service_name = getenv("SERVICE");
}

const char* cred_string(void)
{
  const char* cred;

  /* If there are no more credential strings, fail. */
  if (!credptr || credptr >= bufend) exit(111);
  cred = credptr;
  credptr = strnul(credptr);
  return cred;
}
