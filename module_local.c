/* cvm/module_local.c - Local CVM server module loop
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
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <msg/msg.h>
#include <net/socket.h>

#include "module.h"

static const char* path;
static int sock;
static int conn;

static int read_input(void)
{
  unsigned rd;
  if ((conn = socket_acceptu(sock)) == -1) return CVME_IO;
  for (cvm_module_inbuflen = 0;
       cvm_module_inbuflen < BUFSIZE;
       cvm_module_inbuflen += rd) {
    if ((rd = read(conn, cvm_module_inbuffer+cvm_module_inbuflen,
		   BUFSIZE-cvm_module_inbuflen)) == 0)
      break;
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
  for (written = 0; written < cvm_module_outbuflen; written += wr) {
    if ((wr = write(conn, cvm_module_outbuffer+written,
		    cvm_module_outbuflen-written)) == 0)
      break;
    if (wr == (unsigned)-1) break;
  }
  close(conn);
}

static void exitfn()
{
  unlink(path);
  cvm_module_log_shutdown();
  exit(0);
}

static int make_socket(void)
{
  mode_t old_umask;
  mode_t mode = 0777;
  uid_t owner = -1;
  gid_t group = -1;
  const char* tmp;
  char* end;
  struct passwd* pw;
  struct group* gr;

  if ((tmp = getenv("CVM_SOCKET_MODE")) != 0)
    mode = strtoul(tmp, 0, 8);
  if ((tmp = getenv("CVM_SOCKET_OWNER")) != 0) {
    owner = strtoul(tmp, &end, 10);
    if (*end != 0) {
      if ((pw = getpwnam(tmp)) == 0) {
	error1sys("getpwnam failed");
	return CVME_IO;
      }
      owner = pw->pw_uid;
      group = pw->pw_gid;
    }
  }
  if ((tmp = getenv("CVM_SOCKET_GROUP")) != 0) {
    group = strtoul(tmp, &end, 10);
    if (*end != 0) {
      if ((gr = getgrnam(tmp)) == 0) {
	error1sys("getgrnam failed");
	return CVME_IO;
      }
      group = gr->gr_gid;
    }
  }

  old_umask = umask((mode & 0777) ^ 0777);
  if ((sock = socket_unixstr()) == -1)
    error1sys("Could not create socket");
  else if (!socket_bindu(sock, path))
    error1sys("Could not bind socket");
  else if (chmod(path, mode) == -1)
    error1sys("Could not change socket permission");
  else if (chown(path, owner, group) == -1)
    error1sys("Could not change socket ownership");
  else if (!socket_listen(sock, 1))
    error1sys("Could not listen on socket");
  else {
    umask(old_umask);
    return 0;
  }
  return CVME_IO;
}

extern void usage(void);

int local_main(const char* p)
{
  int code;
  
  path = p;
  
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, exitfn);
  signal(SIGTERM, exitfn);

  if ((code = make_socket()) != 0) return code;
  if ((code = cvm_module_init()) != 0) return code;
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
