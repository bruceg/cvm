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
  char* ptr;
  
  ptr = tmp;
  switch (outbuffer[0]) {
  case 0: status = '+'; break;
  case CVME_PERMFAIL: status = '-'; break;
  default: status = '?'; break;
  }
  *ptr++ = status;

  *ptr++ = ' ';
  memcpy(ptr, cvm_account_name.s, cvm_account_name.len);
  ptr += cvm_account_name.len;

  *ptr++ = '@';
  memcpy(ptr, cvm_account_domain.s, cvm_account_domain.len);
  ptr += cvm_account_domain.len;
  
  *ptr++ = '\n';
  *ptr = 0;
  write(1, tmp, ptr-tmp);
}
