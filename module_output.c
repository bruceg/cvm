/* cvm/module_output.c - Response formatting
 * Copyright (C)2006  Bruce Guenter <bruce@untroubled.org>
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
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "module.h"
#include "protocol.h"

unsigned char cvm_module_outbuffer[BUFSIZE];
unsigned cvm_module_outbuflen;
static unsigned char* outbufptr;

static int v1fact(unsigned number, const char* data, unsigned len)
{
  /* Always leave room for a trailing NUL. */
  if (cvm_module_outbuflen + len + 3 > BUFSIZE) {
    cvm_module_outbuflen = BUFSIZE;
    return 0;
  }
  cvm_module_outbuflen += len + 2;
  *outbufptr++ = number;
  memcpy(outbufptr, data, len);
  outbufptr += len;
  *outbufptr++ = 0;
  return 1;
}

static int v2fact(unsigned number, const char* data, unsigned len)
{
  /* Always leave room for a trailing zero type byte. */
  if (cvm_module_outbuflen + len + 3 > BUFSIZE) {
    cvm_module_outbuflen = BUFSIZE;
    return 0;
  }
  cvm_module_outbuflen += len + 2;
  *outbufptr++ = number;
  *outbufptr++ = len;
  memcpy(outbufptr, data, len);
  outbufptr += len;
  return 1;
}

static int (*fact)(unsigned,const char*,unsigned);

static void cvm1_fact_start(void)
{
  fact = v1fact;
  cvm_module_outbuflen = 1;
  outbufptr = cvm_module_outbuffer + 1;
}

static void cvm2_fact_start(void)
{
  fact = v2fact;
  cvm_module_outbuflen = 0;
  outbufptr = cvm_module_outbuffer;
  v2fact(0, cvm_module_inbuffer+2, cvm_module_inbuffer[1]);
}

void cvm_module_fact_start(void)
{
  if (cvm_module_inbuffer[0] == CVM2_PROTOCOL)
    cvm2_fact_start();
  else
    cvm1_fact_start();
}

int cvm_module_fact_str(unsigned number, const char* data)
{
  if (!data) return 0;
  return fact(number, data, strlen(data));
}

void cvm_module_fact_end(unsigned code)
{
  if (cvm_module_outbuflen >= BUFSIZE)
    code = CVME_BAD_MODDATA;
  cvm_module_outbuffer[0] = code;
  if (code == 0 || cvm_module_inbuffer[0] == CVM2_PROTOCOL) {
    *outbufptr++ = 0;
    ++cvm_module_outbuflen;
  }
  else {
    cvm_module_outbuflen = 1;
  }
}

int cvm_module_fact_uint(unsigned number, unsigned long data)
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
