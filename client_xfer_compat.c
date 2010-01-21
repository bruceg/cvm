/* cvm/client_xfer_compat.c - CVM client command transmission wrappers
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
#include <sys/types.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sysdeps.h>

#include "v1client.h"

static unsigned wrapper(const char* module,
			unsigned char buffer[CVM_BUFSIZE],
			unsigned* buflen,
			unsigned (*fn)(const char* module,
				       const struct cvm_packet* request,
				       struct cvm_packet* response))
{
  struct cvm_packet request;
  struct cvm_packet response;
  unsigned result;

  memcpy(request.data, buffer, sizeof buffer);
  request.length = *buflen;
  result = fn(module, &request, &response);
  memcpy(buffer, response.data, sizeof buffer);
  *buflen = response.length;
  return result;
}


unsigned cvm_xfer_command(const char* module,
			  unsigned char buffer[CVM_BUFSIZE],
			  unsigned* buflen)
{
  return wrapper(module, buffer, buflen, cvm_xfer_command_packets);
}

unsigned cvm_xfer_local(const char* module,
			unsigned char buffer[CVM_BUFSIZE],
			unsigned* buflen)
{
  return wrapper(module, buffer, buflen, cvm_xfer_local_packets);
}

unsigned cvm_xfer_udp(const char* module,
		      unsigned char buffer[CVM_BUFSIZE],
		      unsigned* buflen)
{
  return wrapper(module, buffer, buflen, cvm_xfer_udp_packets);
}
