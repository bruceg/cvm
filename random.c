/* cvm/random.c - CVM random number generation
 * Copyright (C)2008  Bruce Guenter <bruce@untroubled.org>
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

/* Derived from dns_random.c from djbdns-1.05, which was made public
 * domain as per http://cr.yp.to/distributors.html on 2007-12-28. */

#include <string.h>
#include <systime.h>
#include <unistd.h>
#include <uint32.h>
#include "random.h"

static uint32 seed[32];
static uint32 in[12];
static uint32 out[8];
static int outleft = 0;

#define ROTATE(x,b) (((x) << (b)) | ((x) >> (32 - (b))))
#define MUSH(i,b) x = t[i] += (((x ^ seed[i]) + sum) ^ ROTATE(x,b));

static void surf(void)
{
  uint32 t[12]; uint32 x; uint32 sum = 0;
  int r; int i; int loop;

  for (i = 0;i < 12;++i) t[i] = in[i] ^ seed[12 + i];
  for (i = 0;i < 8;++i) out[i] = seed[24 + i];
  x = t[11];
  for (loop = 0;loop < 2;++loop) {
    for (r = 0;r < 16;++r) {
      sum += 0x9e3779b9;
      MUSH(0,5) MUSH(1,7) MUSH(2,9) MUSH(3,13)
      MUSH(4,5) MUSH(5,7) MUSH(6,9) MUSH(7,13)
      MUSH(8,5) MUSH(9,7) MUSH(10,9) MUSH(11,13)
    }
    for (i = 0;i < 8;++i) out[i] ^= t[i + 4];
  }
}

void cvm_random_init(void)
{
  int i;
  struct timeval tv;
  uint32 data[32];

  for (i = 0; i < 32; ++i)
    seed[i] += data[i];

  gettimeofday(&tv, 0);
  in[4] = tv.tv_sec;
  in[5] = tv.tv_usec;
  in[6] = getpid();
  in[7] = getppid();
}

unsigned int cvm_random(unsigned int n)
{
  if (!n)
    return 0;

  if (outleft == 0) {
    /* 8 * 2^32^4 period */
    if (++in[0] == 0)
      if (++in[1] == 0)
	if (++in[2] == 0)
	  ++in[3];
    surf();
    outleft = 8;
  }

  return out[--outleft] % n;
}
