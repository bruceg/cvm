#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "socket/socket.h"
#include "module.h"

static int sock;
static ipv4addr ip;
static unsigned short port;

static int read_input(void)
{
  inbuflen = socket_recv4(sock, inbuffer, BUFSIZE, ip, &port);
  if (inbuflen == (unsigned)-1) return CVME_IO;
  return 0;
}

static void write_output(void)
{
  socket_send4(sock, outbuffer, outbuflen, ip, port);
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
  memcpy(ip, he->h_addr_list[0], 4);
  if ((port = strtoul(argv[2], &tmp, 10)) == 0 ||
      port >= 0xffff || *tmp != 0) usage();
  if ((sock = socket_udp()) == -1) perror("socket");
  if (!socket_bind4(sock, ip, port)) perror("bind");

  log_startup();
  for (;;) {
    if ((code = read_input()) != 0) continue;
    code = handle_request();
    fact_end(code);
    log_request();
    write_output();
  }
}
