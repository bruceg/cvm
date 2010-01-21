/* cvm/module_log.c - CVM server module logging functions
 * Copyright (C) 2010  Bruce Guenter <bruce@untroubled.org>
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

void cvm_module_log_startup(void)
{
  write(1, "Starting.\n", 10);
}

void cvm_module_log_shutdown(void)
{
  write(1, "Stopping.\n", 10);
}

void cvm_module_log_request(void)
{
  char buf[BUFSIZE+4];
  char* ptr;
  
  ptr = buf;
  switch (cvm_module_outbuffer[0]) {
  case 0: *ptr++ = '+'; break;
  case CVME_PERMFAIL: *ptr++ = '-'; break;
  default: *ptr++ = '?'; break;
  }
  *ptr++ = ' ';
  if (cvm_module_credentials[CVM_CRED_ACCOUNT].s != 0) {
    memcpy(ptr,
	   cvm_module_credentials[CVM_CRED_ACCOUNT].s,
	   cvm_module_credentials[CVM_CRED_ACCOUNT].len);
    ptr += cvm_module_credentials[CVM_CRED_ACCOUNT].len;
  }
  if (cvm_module_credentials[CVM_CRED_DOMAIN].s != 0) {
    *ptr++ = '@';
    memcpy(ptr,
	   cvm_module_credentials[CVM_CRED_DOMAIN].s,
	   cvm_module_credentials[CVM_CRED_DOMAIN].len);
    ptr += cvm_module_credentials[CVM_CRED_DOMAIN].len;
  }
  *ptr++ = '\n';
  *ptr = 0;
  write(1, buf, ptr-buf);
}
