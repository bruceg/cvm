/* cvm/client.c - CVM client library
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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "socket/socket.h"
#include "fork.h"
#include "client.h"
#include "iopoll.h"

const char* cvm_account_split_chars = "@";

#define BUFSIZE 512
static char buffer[BUFSIZE];
static unsigned buflen;
static pid_t pid = -1;

/* Buffer management code ****************************************************/
static int parse_buffer(void)
{
  if (buflen < 3) return CVME_BAD_MODDATA;
  if (buffer[buflen-1] != 0 || buffer[buflen-2] != 0) return CVME_BAD_MODDATA;
  if (cvm_fact_str(CVM_FACT_USERNAME, &cvm_fact_username) ||
      cvm_fact_uint(CVM_FACT_USERID, &cvm_fact_userid) ||
      cvm_fact_uint(CVM_FACT_GROUPID, &cvm_fact_groupid) ||
      cvm_fact_str(CVM_FACT_DIRECTORY, &cvm_fact_directory))
    return CVME_BAD_MODDATA;
  cvm_fact_str(CVM_FACT_SHELL, &cvm_fact_shell);
  cvm_fact_str(CVM_FACT_REALNAME, &cvm_fact_realname);
  cvm_fact_str(CVM_FACT_GROUPNAME, &cvm_fact_groupname);
  cvm_fact_str(CVM_FACT_SYS_USERNAME, &cvm_fact_sys_username);
  cvm_fact_str(CVM_FACT_SYS_DIRECTORY, &cvm_fact_sys_directory);
  cvm_fact_str(CVM_FACT_DOMAIN, &cvm_fact_domain);
  return 0;
}

static char* buffer_add(char* ptr, const char* str, unsigned len)
{
  if (ptr - buffer + len + 1 >= BUFSIZE-1) return 0;
  memcpy(ptr, str, len);
  ptr[len] = 0;
  return ptr + len + 1;
}

static unsigned build_buffer(const char* account, const char* domain,
			     const char** credentials, int parse_domain)
{
  char* ptr;
  unsigned i;
  unsigned actlen;
  
  buffer[0] = CVM_PROTOCOL;
  ptr = buffer + 1;

  actlen = strlen(account);
  if (parse_domain) {
    const char* sc;
    if ((sc = getenv("CVM_ACCOUNT_SPLIT_CHARS")) == 0)
      sc = cvm_account_split_chars;
    i = strlen(account);
    while (i-- > 0) {
      if (strchr(sc, account[i]) != 0) {
	domain = account + i + 1;
	actlen = i - 1;
	break;
      }
    }
  }
  
  if ((ptr = buffer_add(ptr, account, actlen)) == 0) return 0;
  if ((ptr = buffer_add(ptr, domain, strlen(domain))) == 0) return 0;

  for (i = 0; credentials[i] != 0; i++)
    if ((ptr = buffer_add(ptr, credentials[i], strlen(credentials[i]))) == 0)
      return 0;

  *ptr++ = 0;
  buflen = ptr - buffer;
  return 1;
}

int cvm_fact_str(int number, const char** data)
{
  static char* ptr = 0;
  static int last_number = -1;
  
  if (!ptr || number != last_number)
    ptr = buffer+1;
  last_number = number;
  
  while (*ptr) {
    char* tmp = ptr;
    ptr += strlen(ptr) + 1;
    if (*tmp == (char)number) {
      *data = tmp + 1;
      return 0;
    }
  }
  return CVME_NOFACT;
}

int cvm_fact_uint(int number, unsigned long* data)
{
  const char* str;
  unsigned long i;
  int err;
  
  if ((err = cvm_fact_str(number, &str)) != 0) return err;

  for (i = 0; *str >= '0' && *str <= '9'; ++str) {
    unsigned long tmp = i;
    i *= 10;
    if (i < tmp) return CVME_BAD_MODDATA;
    i += *str - '0';
  }
  if (*str) return CVME_BAD_MODDATA;
  *data = i;
  return 0;
}

/* Command module execution **************************************************/
static int pipefork(const char* cmd, int pipes[2])
{
  int pipe1[2];
  int pipe2[2];
  
  if (pipe(pipe1) == -1 || pipe(pipe2) == -2) return 0;
  pid = fork();
  switch (pid) {
  case -1:
    return 0;
  case 0:
    close(0);
    close(pipe1[1]);
    dup2(pipe1[0], 0);
    close(pipe1[0]);
    close(1);
    close(pipe2[0]);
    dup2(pipe2[1], 1);
    close(pipe2[1]);
    execlp(cmd, cmd, 0);
    exit(1);
  default:
    close(pipe1[0]);
    pipes[0] = pipe1[1];
    close(pipe2[1]);
    pipes[1] = pipe2[0];
    return 1;
  }
}

static void killit(void)
{
  if (pid != -1)
    kill(pid, SIGTERM);
}

static int waitforit(void)
{
  int status;
  pid_t tmp;
  while ((tmp = wait(&status)) != -1) {
    if (tmp == pid)
      return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
  }
  return 1;
}

static int write_buffer(int fd)
{
  char* ptr;
  size_t wr;

  for (ptr = buffer; buflen; buflen -= wr, ptr += wr) {
    wr = write(fd, ptr, buflen);
    if (wr == 0 || wr == (unsigned)-1) return 0;
  }
  return 1;
}

static int read_buffer(int fd)
{
  char* ptr;
  size_t rd;

  for (ptr = buffer, buflen = 0; buflen < BUFSIZE; ptr += rd, buflen += rd) {
    rd = read(fd, ptr, BUFSIZE-buflen);
    if (rd == (unsigned)-1) return 0;
    if (rd == 0) break;
  }
  return 1;
}

static int cvm_command(const char* module)
{
  int pipes[2];
  int result;

  if (!pipefork(module, pipes)) return 1;
  
  if (!write_buffer(pipes[0]) ||
      close(pipes[0]) == -1 ||
      !read_buffer(pipes[1]) ||
      close(pipes[1]) == -1) {
    killit();
    if ((result = waitforit()) != 0) return result;
    return CVME_IO;
  }

  return waitforit();
}

/* UDP module invocation *****************************************************/
static int udp_sendrecv(int sock, ipv4addr ip, unsigned short port)
{
  int timeout;
  int try;
  iopoll_fd ifd;

  ifd.fd = sock;
  ifd.events = IOPOLL_READ;
  for (timeout = 2, try = 0; try < 4; timeout *= 2, ++try) {
    if ((unsigned)socket_send4(sock, buffer, buflen, ip, port) != buflen)
      return 0;
    if (iopoll(&ifd, 1, timeout*1000) != 0)
      return (buflen = socket_recv4(sock, buffer, BUFSIZE, ip, &port)) !=
	(unsigned)-1;
  }
  return 0;
}

static int cvm_udp(const char* hostport)
{
  static char* hostname;
  char* portstr;
  unsigned short port;
  int sock;
  struct hostent* he;
  ipv4addr ip;
  
  if ((portstr = strchr(hostport, ':')) == 0) return 1;
  if (hostname) free(hostname);
  hostname = malloc(portstr-hostport+1);
  memcpy(hostname, hostport, portstr-hostport);
  hostname[portstr-hostport] = 0;
  port = strtoul(portstr+1, &portstr, 10);
  if (*portstr != 0) return 1;
  if ((he = gethostbyname(hostname)) == 0) return 1;
  memcpy(ip, he->h_addr_list[0], 4);
  
  if ((sock = socket_udp()) == -1) return CVME_IO;
  if (!socket_connect4(sock, ip, port) ||
      !udp_sendrecv(sock, ip, port)) {
    close(sock);
    return CVME_IO;
  }
  close(sock);
  return buffer[0];
}

/* UNIX local-domain socket module invocation ********************************/
static int cvm_local(const char* path)
{
  int sock;
  int result;
  unsigned io;
  unsigned done;
  void (*oldsig)(int);
  
  oldsig = signal(SIGPIPE, SIG_IGN);
  result = CVME_IO;
  if ((sock = socket_unixstr()) != -1 &&
      socket_connectu(sock, path)) {
    for (done = 0; done < buflen; done += io) {
      if ((io = write(sock, buffer+done, buflen-done)) == 0) break;
      if (io == (unsigned)-1) break;
    }
    socket_shutdown(sock, 0, 1);
    if (done >= buflen) {
      for (done = 0; done < BUFSIZE; done += io) {
	if ((io = read(sock, buffer+done, BUFSIZE-done)) == 0) break;
	if (io == (unsigned)-1) done = BUFSIZE+1;
      }
      if (done <= BUFSIZE) {
	buflen = done;
	result = 0;
      }
    }
  }
  close(sock);
  signal(SIGPIPE, oldsig);
  return result;
}

/* Top-level wrapper *********************************************************/
int cvm_authenticate(const char* module, const char* account,
		     const char* domain, const char** credentials,
		     int parse_domain)
{
  int result;
  if (domain == 0) domain = "";
  if (!build_buffer(account, domain, credentials, parse_domain))
    return CVME_GENERAL;
  if (!memcmp(module, "cvm-udp:", 8))
    result = cvm_udp(module+8);
  else if (!memcmp(module, "cvm-local:", 10))
    result = cvm_local(module+10);
  else {
    if (!memcmp(module, "cvm-command:", 12)) module += 12;
    result = cvm_command(module);
  }
  if (result != 0) return result;
  if (buffer[0]) return buffer[0];
  return parse_buffer();
}
