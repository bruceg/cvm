/* cvm/sql-query.c - SQL query parsing and insertion framework.
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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <cvm/module.h>
#include "sql.h"
#include <str/str.h>

#define QUOTE '\''
#define BACKSLASH '\\'

static int str_cats_quoted(str* s, const char* ptr)
{
  if (!str_catc(s, QUOTE)) return 0;
  for (; *ptr; ++ptr) {
    switch (*ptr) {
    case QUOTE: if (!str_catc(s, QUOTE)) return 0; break;
    case BACKSLASH: if (!str_catc(s, BACKSLASH)) return 0; break;
    }
    str_catc(s, *ptr);
  }
  return str_catc(s, QUOTE);
}

int sql_query_validate(const char* template)
{
  while ((template = strchr(template, '$')) != 0) {
    ++template;
    switch (*template) {
    case '$':
      ++template;
      break;
    case '{':
      ++template;
      if ((template = strchr(template, '}')) == 0) return 0;
      ++template;
    default:
      while (isalnum(*template) || *template == '_') ++template;
    }
  }
  return 1;
}

int sql_query_build(const char* template, str* q)
{
  static str name;
  const char* ptr;
  if (!str_truncate(q, 0)) return 0;
  while ((ptr = strchr(template, '$')) != 0) {
    if (!str_catb(q, template, ptr - template)) return 0;
    template = ptr + 1;
    switch (*template) {
    case '$':
      ++template;
      if (!str_truncate(&name, 0)) return 0;
      break;
    case '{':
      ++template;
      if ((ptr = strchr(template, '}')) == 0) return 0;
      if (!str_copyb(&name, template, ptr-template)) return 0;
      template = ptr + 1;
      break;
    default:
      if (!str_truncate(&name, 0)) return 0;
      while (isalnum(*template) || *template == '_')
	if (!str_catc(&name, *template++)) return 0;
    }
    if (name.len == 0) {
      if (!str_catc(q, '$')) return 0;
    }
    else {
      if (str_diffs(&name, "account") == 0)
	ptr = cvm_account_name;
      else if (str_diffs(&name, "domain") == 0)
	ptr = cvm_account_domain;
      else
	ptr = getenv(name.s);
      if (ptr) if (!str_cats_quoted(q, ptr)) return 0;
    }
  }
  if (!str_cats(q, template)) return 0;
  return 1;
}
