/* cvm/module_udp.c - UDP CVM server module main routine
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
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/socket.h>

#include "module.h"

static int sock;
static ipv4addr ip;
static ipv4port port;

static int read_input(void)
{
  inbuflen = socket_recv4(sock, inbuffer, BUFSIZE, &ip, &port);
  if (inbuflen == (unsigned)-1) return CVME_IO;
  return 0;
}

static void write_output(void)
{
  socket_send4(sock, outbuffer, outbuflen, &ip, port);
}

static void exitfn()
{
  log_shutdown();
  exit(0);
}

static const char usagestr[] =
"usage: cvm-module-udp IP PORT\n";

static void usage(void)
{
  write(2, usagestr, sizeof usagestr);
  exit(1);
}

int main(int argc, char** argv)
{
  int code;
  struct hostent* he;
  char* tmp;
  
  if (argc != 3) usage();

  signal(SIGINT, exitfn);
  signal(SIGTERM, exitfn);
  
  if ((he = gethostbyname(argv[1])) == 0) usage();
  memcpy(&ip, he->h_addr_list[0], 4);
  if ((port = strtoul(argv[2], &tmp, 10)) == 0 ||
      port >= 0xffff || *tmp != 0) usage();
  if ((sock = socket_udp()) == -1) perror("socket");
  if (!socket_bind4(sock, &ip, port)) perror("bind");
  if ((code = cvm_auth_init()) != 0) return code;
  log_startup();

  code = 0;
  do {
    if ((code = read_input()) != 0) continue;
    code = handle_request();
    cvm_fact_end(code & CVME_MASK);
    log_request();
    write_output();
  } while ((code & CVME_FATAL) == 0);
  cvm_auth_stop();
  return 0;
}
