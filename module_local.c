/* cvm/module_local.c - Local CVM server module loop
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <net/socket.h>

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
    if (rd == (unsigned)-1) {
      close(conn);
      return CVME_IO;
    }
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

extern void usage(void);

int local_main(const char* p)
{
  int code;
  mode_t old_umask;
  
  path = p;
  
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, exitfn);
  signal(SIGTERM, exitfn);
  
  if ((sock = socket_unixstr()) == -1) perror("socket");
  old_umask = umask(0);
  if (!socket_bindu(sock, path)) perror("bind");
  umask(old_umask);
  if (!socket_listen(sock, 1)) perror("listen");
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
