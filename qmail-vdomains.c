/* qmail-vdomains.c - qmail virtualdomains lookup routines
 * Copyright (C) 2004  Bruce Guenter <bruceg@em.ca>
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
#include <sys/stat.h>

#include <dict/dict.h>
#include <dict/load.h>
#include <str/str.h>

#include "qmail.h"

static dict vdomains;
static struct stat vdomains_stat;
static str vdomains_path;

static int map_lower(str* s)
{
  str_lower(s);
  return 1;
}

static int load_vdomains(const struct stat* s)
{
  if (s) {
    // FIXME: obuf_putsflush(&errbuf, "Reloading virtualdomains\n");
    vdomains_stat = *s;
  }
  else {
    // FIXME: obuf_putsflush(&errbuf, "Loading virtualdomains\n");
    if (stat(vdomains_path.s, &vdomains_stat) == -1) return 0;
  }
  dict_free(&vdomains, dict_str_free);
  return dict_load_map(&vdomains, vdomains_path.s, 0, ':', map_lower, 0);
}

int qmail_vdomains_reinit(void)
{
  struct stat s;
  if (stat(vdomains_path.s, &s) != 0)
    return -1;
  if (vdomains_stat.st_mtime != s.st_mtime ||
      vdomains_stat.st_ino != s.st_ino ||
      vdomains_stat.st_size != s.st_size)
    if (!load_vdomains(&s))
      return -1;
  return 0;
}

int qmail_vdomains_init(void)
{
  if (!str_copy2s(&vdomains_path, qmail_root, "/control/virtualdomains"))
    return -1;
  if (!load_vdomains(0))
    return -1;
  return 0;
}

int qmail_vdomains_lookup(const char* d, str* domain, str* prefix)
{
  dict_entry* e;

  if (!str_copys(domain, d))
    return -1;
  str_lower(domain);

  if ((e = dict_get(&vdomains, domain)) == 0) {
    unsigned i;
    while ((i = str_findnext(domain, '.', 1)) != (unsigned)-1) {
      str_lcut(domain, i);
      if ((e = dict_get(&vdomains, domain)) != 0)
	break;
    }
  }
  if (e == 0)
    return 0;
  if (!str_copy(prefix, (str*)e->data))
    return -1;
  return 1;
}
