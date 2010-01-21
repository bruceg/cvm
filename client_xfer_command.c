/* cvm/client_xfer_command.c - CVM client command transmission library
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

static pid_t pid;

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
    execlp(cmd, cmd, NULL);
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
      return WIFEXITED(status) ? WEXITSTATUS(status) : -CVME_IO;
  }
  return -CVME_IO;
}

static int write_buffer(int fd, const unsigned char* buffer, unsigned buflen)
{
  size_t wr;

  while (buflen > 0) {
    wr = write(fd, buffer, buflen);
    if (wr == 0 || wr == (unsigned)-1) return 0;
    buflen -= wr;
    buffer += wr;
  }
  return 1;
}

static unsigned read_buffer(int fd, unsigned char* buffer)
{
  size_t rd;
  unsigned buflen;

  for (buflen = 0; buflen < CVM_BUFSIZE; buffer += rd, buflen += rd) {
    rd = read(fd, buffer, CVM_BUFSIZE-buflen);
    if (rd == (unsigned)-1) return 0;
    if (rd == 0) break;
  }
  return buflen;
}

unsigned cvm_xfer_command_packets(const char* module,
				  const struct cvm_packet* request,
				  struct cvm_packet* response)
{
  int pipes[2];
  int result;

  if (!pipefork(module, pipes)) return CVME_IO;
  
  if (!write_buffer(pipes[0], request->data, request->length) ||
      close(pipes[0]) == -1 ||
      (response->length = read_buffer(pipes[1], response->data)) == 0 ||
      close(pipes[1]) == -1) {
    killit();
    if ((result = waitforit()) < 0)
      return -result;
    return CVME_IO;
  }

  if ((result = waitforit()) < 0)
    return -result;
  response->data[0] = result;
  return 0;
}
