/* cvm/module_log.c - CVM server module logging functions
 * Copyright (C) 2001  Bruce Guenter <bruceg@em.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
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
