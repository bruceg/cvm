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
