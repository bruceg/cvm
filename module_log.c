#include <string.h>
#include <unistd.h>
#include "module.h"

void log_startup(void)
{
  write(1, "Starting.\n", 10);
}

void log_shutdown(void)
{
  write(1, "Stopping.\n", 10);
}

void log_request(void)
{
  char tmp[BUFSIZE+4];
  char status;
  unsigned long account_len;
  
  switch (outbuffer[0]) {
  case 0: status = '+'; break;
  case CVME_PERMFAIL: status = '-'; break;
  default: status = '?'; break;
  }
  account_len = strlen(account_name);
  tmp[0] = status;
  tmp[1] = ' ';
  memcpy(tmp+2, account_name, account_len);
  tmp[account_len+2] = '\n';
  tmp[account_len+3] = 0;
  write(1, tmp, strlen(tmp));
}
