/* qmail-domains.c - qmail locals/virtualdomains lookup routines
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
#include <errno.h>
#include <sys/stat.h>

#include <dict/dict.h>
#include <dict/load.h>
#include <str/str.h>

#include "qmail.h"

static dict vdomains;
static struct stat vdomains_stat;
static str vdomains_path;

static dict locals;
static struct stat locals_stat;
static str locals_path;

static int assume_local = 0;

static int map_lower(str* s)
{
  str_lower(s);
  return 1;
}

static int stat_changed(const char* path, const struct stat* orig,
			struct stat* curr)
{
  if (stat(path, curr) != 0)
    return -1;
  if (orig->st_mtime != curr->st_mtime
      || orig->st_ino != curr->st_ino
      || orig->st_size != curr->st_size)
    return 1;
  return 0;
}

static int load_dict(const char* path, struct stat* oldstat,
		     dict* dictp, void (*free_fn)(void*),
		     int (*load_fn)(void))
{
  struct stat s;
  switch (stat_changed(path, oldstat, &s)) {
  case -1:
    if (errno != ENOENT)
      return 0;
    oldstat->st_mtime = 0;
    oldstat->st_ino = 0;
    oldstat->st_size = 0;
    dict_free(dictp, free_fn);
    return 1;
  case 0:
    return 1;
  }
  // FIXME: obuf_putsflush(&errbuf, "Reloading *path*\n");
  *oldstat = s;
  dict_free(dictp, free_fn);
  return load_fn();
}

static int _load_vdomains(void)
{
  return dict_load_map(&vdomains, vdomains_path.s, 0, ':', map_lower, 0);
}

static int load_vdomains(void)
{
  return load_dict(vdomains_path.s, &vdomains_stat, &vdomains, dict_str_free, _load_vdomains);
}

static int _load_locals(void)
{
  return dict_load_list(&locals, locals_path.s, 0, map_lower);
}

static int load_locals(void)
{
  return load_dict(locals_path.s, &locals_stat, &locals, 0, _load_locals);
}

int qmail_domains_reinit(void)
{
  if (!load_locals()
      || !load_vdomains())
    return -1;
  
  return 0;
}

int qmail_domains_init(void)
{
  assume_local = getenv("CVM_QMAIL_ASSUME_LOCAL") != 0;

  if (!str_copy2s(&vdomains_path, qmail_root, "/control/virtualdomains")
      || !str_copy2s(&locals_path, qmail_root, "/control/locals"))
    return -1;

  if (!load_locals()
      || !load_vdomains())
    return -1;

  return 0;
}

int qmail_domains_lookup(const str* d, str* domain, str* prefix)
{
  dict_entry* e;

  if (!str_copy(domain, d))
    return -1;
  str_lower(domain);

  if ((e = dict_get(&locals, domain)) != 0)
    return str_copys(prefix, "") ? 1 : -1;

  if ((e = dict_get(&vdomains, domain)) == 0) {
    unsigned i;
    while ((i = str_findnext(domain, '.', 1)) != (unsigned)-1) {
      str_lcut(domain, i);
      if ((e = dict_get(&vdomains, domain)) != 0)
	break;
    }
  }
  if (e == 0) {
    if (assume_local) {
      if (!str_copys(prefix, "")) return -1;
      if (!str_copy(domain, d)) return -1;
      str_lower(domain);
      return 1;
    }
    return 0;
  }
  if (!str_copy(prefix, (str*)e->data))
    return -1;
  return 1;
}
