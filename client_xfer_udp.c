/* cvm/client_xfer_udp.c - CVM client UDP transmission library
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
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sysdeps.h>
#include <net/socket.h>

#include "v1client.h"
#include "protocol.h"

/* UDP module invocation *****************************************************/
static int udp_sendrecv(int sock, ipv4addr* ip, ipv4port port,
			const struct cvm_packet* request,
			struct cvm_packet* response)
{
  int timeout;
  int try;
  iopoll_fd ifd;

  ifd.fd = sock;
  ifd.events = IOPOLL_READ;
  for (timeout = 2, try = 0; try < 4; timeout *= 2, ++try) {
    if ((unsigned)socket_send4(sock, request->data, request->length,
			       ip, port) != request->length)
      return 0;
    if (iopoll(&ifd, 1, timeout*1000) != 0)
      return (response->length = socket_recv4(sock, response->data,
					      CVM_BUFSIZE, ip,
					      &port)) != (unsigned)-1;
  }
  return 0;
}

unsigned cvm_xfer_udp_packets(const char* hostport,
			      const struct cvm_packet* request,
			      struct cvm_packet* response)
{
  static char* hostname;
  char* portstr;
  ipv4port port;
  int sock;
  struct hostent* he;
  ipv4addr ip;
  
  if ((portstr = strchr(hostport, ':')) == 0) return CVME_GENERAL;
  if (hostname) free(hostname);
  hostname = malloc(portstr-hostport+1);
  memcpy(hostname, hostport, portstr-hostport);
  hostname[portstr-hostport] = 0;
  port = strtoul(portstr+1, &portstr, 10);
  if (*portstr != 0) return CVME_GENERAL;
  if ((he = gethostbyname(hostname)) == 0) return CVME_GENERAL;
  memcpy(&ip, he->h_addr_list[0], 4);
  
  if ((sock = socket_udp()) == -1) return CVME_IO;
  if (!udp_sendrecv(sock, &ip, port, request, response)) {
    close(sock);
    return CVME_IO;
  }
  close(sock);
  return 0;
}
