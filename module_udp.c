/* cvm/module_udp.c - UDP CVM server module loop
 * Copyright (C) 2005  Bruce Guenter <bruceg@em.ca>
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
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <msg/msg.h>
#include <net/socket.h>

#include "module.h"

static int sock;
static ipv4addr ip;
static ipv4port port;

static int read_input(void)
{
  cvm_module_inbuflen = socket_recv4(sock, cvm_module_inbuffer, BUFSIZE,
				     &ip, &port);
  if (cvm_module_inbuflen == (unsigned)-1) return CVME_IO;
  return 0;
}

static void write_output(void)
{
  socket_send4(sock, cvm_module_outbuffer, cvm_module_outbuflen, &ip, port);
}

static void exitfn()
{
  cvm_module_log_shutdown();
  exit(0);
}

extern void usage(void);

int udp_main(const char* hostname, const char* portname)
{
  int code;
  struct hostent* he;
  char* tmp;
  
  signal(SIGINT, exitfn);
  signal(SIGTERM, exitfn);
  
  if ((he = gethostbyname(hostname)) == 0) usage();
  memcpy(&ip, he->h_addr_list[0], 4);
  if ((port = strtoul(portname, &tmp, 10)) == 0 ||
      port >= 0xffff || *tmp != 0) usage();
  if ((sock = socket_udp()) == -1) {
    error1sys("Could not create socket");
    return CVME_IO;
  }
  if (!socket_bind4(sock, &ip, port)) {
    error1sys("Could not bind socket");
    return CVME_IO;
  }
  if ((code = cvm_module_init()) != 0)
    return code;
  cvm_module_log_startup();

  code = 0;
  do {
    if ((code = read_input()) != 0) continue;
    code = cvm_module_handle_request();
    cvm_module_fact_end(code & CVME_MASK);
    cvm_module_log_request();
    write_output();
  } while ((code & CVME_FATAL) == 0);
  cvm_module_stop();
  return 0;
}
