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

#define BUFSIZE 512
static char buffer[BUFSIZE];
static unsigned buflen;
static pid_t pid = -1;

/* Buffer management code ****************************************************/
static int parse_buffer(void)
{
  if (buflen < 3) return 0;
  if (buffer[0]) return buffer[0];
  if (buffer[buflen-1] != 0 || buffer[buflen-2] != 0) return 0;
  if (!fact_str(FACT_USERNAME, &fact_username) ||
      !fact_uint(FACT_USERID, &fact_userid) ||
      !fact_uint(FACT_GROUPID, &fact_groupid) ||
      !fact_str(FACT_DIRECTORY, &fact_directory) ||
      !fact_str(FACT_SHELL, &fact_shell)) return 0;
  fact_str(FACT_REALNAME, &fact_realname);
  fact_str(FACT_GROUPNAME, &fact_groupname);
  fact_str(FACT_SYS_USERNAME, &fact_sys_username);
  fact_str(FACT_SYS_DIRECTORY, &fact_sys_directory);
  return 1;
}

static unsigned build_buffer(const char** credentials)
{
  char* ptr;
  unsigned i;
  unsigned len;

  ptr = buffer;
  for (i = 0; credentials[i]; i++) {
    len = strlen(credentials[i])+1;
    if (ptr-buffer+len >= BUFSIZE-1) return 0;
    memcpy(ptr, credentials[i], len);
    ptr += len;
  }
  *ptr++ = 0;
  buflen = ptr - buffer;
  return 1;
}

int fact_str(int number, const char** data)
{
  char* ptr;
  
  for (ptr = buffer; *ptr; ptr += strlen(ptr)+1) {
    if (*ptr == (char)number) {
      *data = ptr + 1;
      return 1;
    }
  }
  return 0;
}

int fact_uint(int number, unsigned* data)
{
  const char* tmp;
  unsigned i;
  
  if (!fact_str(number, &tmp)) return 0;
  for (i = 0; *tmp >= '0' && *tmp <= '9'; ++tmp)
    i = i * 10 + *tmp - '0';
  if (*tmp) return 0;
  *data = i;
  return 1;
}

/* Command module execution **************************************************/
static int pipefork(const char* cmd, int pipes[2])
{
  int pipe1[2];
  int pipe2[2];
  
  if (pipe(pipe1) == -1 || pipe(pipe2) == -2) return 0;
  pid = vfork();
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
    exit(111);
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
      return WIFEXITED(status) ? WEXITSTATUS(status) : 111;
  }
  return 111;
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

  for (ptr = buffer+1, buflen = 1; buflen < BUFSIZE; ptr += rd, buflen += rd) {
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

  if (!pipefork(module, pipes)) return 111;
  
  if (!write_buffer(pipes[0]) ||
      close(pipes[0]) == -1 ||
      !read_buffer(pipes[1]) ||
      close(pipes[1]) == -1) {
    killit();
    if ((result = waitforit()) != 0) return result;
    return 111;
  }

  buffer[0] = waitforit();
  return buffer[0];
}

/* UDP module invocation *****************************************************/
static int cvm_udp(const char* hostport)
{
  static char* hostname;
  char* portstr;
  unsigned short port;
  int sock;
  struct hostent* he;
  
  if ((portstr = strchr(hostport, ':')) == 0) return 111;
  if (hostname) free(hostname);
  hostname = malloc(portstr-hostport+1);
  memcpy(hostname, hostport, portstr-hostport);
  hostname[portstr-hostport] = 0;
  port = strtoul(portstr+1, &portstr, 10);
  if (*portstr != 0) return 111;
  if ((he = gethostbyname(hostname)) == 0) return 111;
  if ((sock = socket_udp()) == -1) return 111;
  if ((unsigned)socket_send4(sock, buffer, buflen, he->h_addr_list[0], port) !=
      buflen) return 111;
  if ((buflen = socket_recv4(sock, buffer, buflen, he->h_addr_list[0], &port))
      == (unsigned)-1) return 111;
  return buffer[0];
}

/* Top-level wrapper *********************************************************/
int authenticate(const char* module, const char** credentials)
{
  int result;
  if (!build_buffer(credentials)) return 111;
  if (!memcmp(module, "cvm-udp:", 8))
    result = cvm_udp(module+8);
  else
    result = cvm_command(module);
  if (result != 0) return result;
  if (!parse_buffer()) return 111;
  return 0;
}
