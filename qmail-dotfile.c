/* qmail-dotfile.c - qmail dotfile ($HOME/.qmail*) lookup routines
 * Copyright (C)2006  Bruce Guenter <bruce@untroubled.org>
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
#include <sysdeps.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>

#include <str/str.h>

#include "qmail.h"

int qmail_dotfile_exists(const struct qmail_user* user, const char* ext)
{
  static str path;
  struct stat st;
  int split;
  int baselen;

  /* System users are not required to have a .qmail file */
  if (user->dash == 0)
    return ext == 0 || *ext == 0;

  if (!str_copy(&path, &user->homedir)) return -1;
  if (!str_cats(&path, "/.qmail")) return -1;
  baselen = path.len;
  if (!str_catc(&path, user->dash)) return -1;
  if (!str_cat(&path, &user->ext)) return -1;
  if (ext != 0) {
    while (*ext) {
      if (!str_catc(&path, isupper(*ext)
		    ? tolower(*ext)
		    : (*ext == '.')
		    ? ':'
		    : *ext))
	return -1;
      ++ext;
    }
  }

  split = path.len;
  for (;;) {
    if (stat(path.s, &st) == 0)
      return 1;
    if (errno != ENOENT)
      return -1;
    if ((split = str_findprev(&path, '-', split - 1)) == -1
	|| split < baselen)
      break;
    path.len = split + 1;
    if (!str_cats(&path, "default")) return -1;
  }
  return 0;
}
