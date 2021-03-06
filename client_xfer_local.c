/* cvm/client_xfer_local.c - CVM client local transmission library
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
#include <bglibs/sysdeps.h>
#include <unistd.h>

#include <bglibs/socket.h>

#include "v1client.h"
#include "protocol.h"

/* UNIX local-domain socket module invocation ********************************/
unsigned cvm_xfer_local_packets(const char* path,
				const struct cvm_packet* request,
				struct cvm_packet* response)
{
  int sock;
  int result;
  unsigned io;
  unsigned done;
  unsigned len;
  result = CVME_IO;
  response->length = 0;
  if ((sock = socket_unixstr()) != -1 &&
      socket_connectu(sock, path)) {
    for (done = 0, len = request->length; done < len; done += io) {
      if ((io = write(sock, request->data+done, len-done)) == 0) break;
      if (io == (unsigned)-1) break;
    }
    socket_shutdown(sock, 0, 1);
    if (done >= len) {
      for (done = 0; done < CVM_BUFSIZE; done += io) {
	if ((io = read(sock, response->data+done, CVM_BUFSIZE-done)) == 0)
	  break;
	if (io == (unsigned)-1) done = CVM_BUFSIZE+1;
      }
      if (done <= CVM_BUFSIZE) {
	response->length = done;
	result = 0;
      }
    }
  }
  close(sock);
  return result;
}
