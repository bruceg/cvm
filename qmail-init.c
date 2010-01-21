/* qmailn.c - qmail initialization routines
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
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "qmail.h"

const char* qmail_root = "/var/qmail";
const char* qmail_me = 0;
const char* qmail_envnoathost = 0;

static int read_control(const char* control,
			const char** s,
			str* path)
{
  /* Hostnames are realistically limited to 256 bytes.  This is overkill. */
  char buf[4096];
  int fd;
  long rd;
  char* nl;
  char* news;
  long len;
  if (!str_copy3s(path, qmail_root, "/control/", control))
    return -1;
  if ((fd = open(path->s, O_RDONLY)) == -1)
    return (errno == ENOENT) ? 0 : -1;
  rd = read(fd, buf, sizeof buf);
  close(fd);
  if (rd <= 0)
    return rd;
  if ((nl = memchr(buf, '\n', rd)) == 0)
    nl = buf + rd;
  len = nl - buf;
  if ((news = malloc(len + 1)) == 0)
    return -1;
  memcpy(news, buf, len);
  news[len] = 0;
  *s = news;
  return 0;
}

int qmail_init(void)
{
  str path = { 0,0,0 };
  const char* tmp;
  if ((tmp = getenv("QMAIL_ROOT")) != 0)
    qmail_root = tmp;
  if (read_control("envnoathost", &qmail_envnoathost, &path) != 0)
    return -1;
  if (read_control("me", &qmail_me, &path) != 0)
    return -1;
  if (qmail_envnoathost == 0)
    qmail_envnoathost = (qmail_me == 0) ? "envnoathost" : qmail_me;
  if (qmail_me == 0)
    qmail_me = "me";
  str_free(&path);
  return 0;
}
