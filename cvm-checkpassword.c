/* cvm/cvm-checkpassword.c - Checkpassword emulator for CVM
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "client.h"
#include "setenv.h"

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
  const char* tokens[3];
  
  if (argc < 3) {
    printf("usage: cvm-checkpassword cvmodule program [args ...]\n");
    return 1;
  }

  get_data();
  tokens[0] = buffer;
  tokens[1] = pass;
  tokens[2] = 0;
  if ((i = cvm_authenticate(argv[1], tokens)) != 0) return i;

  if (setenv("HOME", cvm_fact_directory, 1) != 0 ||
      setenv("USER", cvm_fact_username, 1) != 0 ||
      (cvm_fact_realname && setenv("NAME", cvm_fact_realname, 1) != 0) ||
      setenv("SHELL", cvm_fact_shell,1) != 0 ||
      chdir(cvm_fact_directory) != 0 ||
      setgid(cvm_fact_groupid) != 0 ||
      setuid(cvm_fact_userid) != 0)
    return 111;

  execvp(argv[2], argv+2);
  return 111;
}
