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
#include <string.h>
#include "module.h"
#include "sql.h"
#include "str/str.h"

static const char* query =
"SELECT password,username,userid,groupid,realname,directory,shell "
"FROM accounts "
"WHERE username=%";
static unsigned query_len1;
static const char* query2;

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

int sql_query_setup(const char* q)
{
  const char* tmp;
  unsigned query_len;

  query = q;
  query_len = strlen(query);
  if ((tmp = strchr(query, '%')) == 0) return CVME_CONFIG;
  query_len1 = tmp - query;
  query2 = tmp + 1;
  return 0;
}

int sql_query_build(str* q)
{
  if (!str_copyb(q, query, query_len1) ||
      !str_cats_quoted(q, cvm_account_name) ||
      !str_cats(q, query2))
    return CVME_GENERAL;
  return 0;
}
