/* cvm/module_command.c - CVM command module main routine
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
#include <unistd.h>
#include "module.h"

static int read_input(void)
{
  size_t rd;

  inbuflen = 0;
  for (inbuflen = 0; inbuflen <= BUFSIZE; inbuflen += rd) {
    if ((rd = read(0, inbuffer+inbuflen, BUFSIZE-inbuflen)) == (unsigned)-1)
      return CVME_IO;
    if (rd == 0) break;
  }
  return 0;
}

static int write_output(void)
{
  size_t wr;
  char* ptr;
  
  for (ptr = outbuffer; outbuflen > 0; outbuflen -= wr, ptr += wr) {
    if ((wr = write(1, ptr, outbuflen)) == (unsigned)-1 || wr == 0)
      return CVME_IO;
  }
  return 0;
}

int main(void)
{
  int code;
  if ((code = read_input()) != 0) return code;
  if ((code = handle_request()) != 0) return code;
  fact_end(code);
  if ((code = write_output()) != 0) return code;
  return 0;
}
