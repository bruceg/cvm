/* cvm/module_local.c - Local CVM server module main routine
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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "socket/socket.h"
#include "module.h"

static const char* path;
static int sock;
static int conn;

static int read_input(void)
{
  unsigned rd;
  if ((conn = socket_acceptu(sock)) == -1) return CVME_IO;
  for (inbuflen = 0; inbuflen < BUFSIZE; inbuflen += rd) {
    if ((rd = read(conn, inbuffer+inbuflen, BUFSIZE-inbuflen)) == 0) break;
    if (rd == (unsigned)-1) return CVME_IO;
  }
  return 0;
}

static void write_output(void)
{
  unsigned wr;
  unsigned written;
  for (written = 0; written < outbuflen; written += wr) {
    if ((wr = write(conn, outbuffer+written, outbuflen-written)) == 0) break;
    if (wr == (unsigned)-1) break;
  }
  close(conn);
}

static void exitfn()
{
  unlink(path);
  log_shutdown();
  exit(0);
}

static const char usagestr[] =
"usage: cvm-module-unix /PATH/TO/SOCKET\n";

static void usage(void)
{
  write(2, usagestr, sizeof usagestr);
  exit(1);
}

int main(int argc, char** argv)
{
  int code;
  
  if (argc != 2) usage();
  path = argv[1];
  
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, exitfn);
  signal(SIGTERM, exitfn);
  
  if ((sock = socket_unixstr()) == -1) perror("socket");
  if (!socket_bindu(sock, path)) perror("bind");
  if (!socket_listen(sock, 1)) perror("listen");
  log_startup();
  
  for (;;) {
    if ((code = read_input()) != 0) continue;
    code = handle_request();
    fact_end(code);
    log_request();
    write_output();
  }
}
