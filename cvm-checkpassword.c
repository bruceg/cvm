/* cvm/cvm-checkpassword.c - Checkpassword emulator for CVM
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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <bglibs/ucspi.h>
#include <bglibs/msg.h>
#include "v2client.h"

const char program[] = "cvm-checkpassword";
const int msg_show_pid = 0;

static char buffer[513];
static char* pass;

void get_data(void)
{
  unsigned buflen;
  unsigned rd;

  for (buflen = 0; buflen < sizeof buffer; buflen += rd) {
    do
      rd = read(3, buffer+buflen, sizeof buffer - buflen);
    while ((rd == (unsigned)-1) && (errno == EINTR));
    if (rd == (unsigned)-1) exit(111);	/* Read error */
    if (rd == 0) break;
  }
  close(3);
  if (buflen >= sizeof buffer) exit(2); /* Buffer too long */
  if ((pass = memchr(buffer, 0, buflen)) == 0) exit(2);	/* No password */
  ++pass;
  if (memchr(pass, 0, buflen-(pass-buffer)) == 0) exit(2); /* No terminator */
}

int main(int argc, char** argv)
{
  int i;
  const char* tokens[2];
  
  if (argc < 3)
    die3(111, "usage: ", program, " cvmodule program [args ...]");

  get_data();
  tokens[0] = pass;
  tokens[1] = 0;
  if ((i = cvm_client_authenticate_password(argv[1], buffer,
					    ucspi_localhost(),
					    pass, 1)) != 0)
    return i;

  if (!cvm_client_setugid()) return 111;
  if (!cvm_client_setenv()) return 111;

  execvp(argv[2], argv+2);
  return 111;
}
