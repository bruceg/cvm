/* cvm/module.c - CVM generic server module main routine
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
#include <stdlib.h>
#include <string.h>

#include <msg/msg.h>

#include "module.h"

const int msg_show_pid = 0;

void usage(void)
{
  die5(1, "Incorrect usage.\n"
       "usage: ", program, " cvm-local:/path/to/socket\n"
       "or:    ", program, " cvm-udp:hostname:port");
}

extern int command_main(void);
extern int local_main(const char*);
extern int udp_main(const char*, const char*);

int main(int argc, char** argv)
{
  cvm_module_init_request();
  if (argc == 1)
    return command_main();
  if (argc == 2) {
    if (memcmp(argv[1], "cvm-local:", 10) == 0)
      return local_main(argv[1] + 10);
    if (memcmp(argv[1], "cvm-udp:", 8) == 0) {
      char* hostname = argv[1] + 8;
      char* port = strchr(hostname, ':');
      if (port == 0) usage();
      *port++ = 0;
      return udp_main(hostname, port);
    }
  }
  usage();
  return 0;
}
