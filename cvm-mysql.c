/* cvm/cvm-mysql.c - MySQL CVM
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
#include <mysql/mysql.h>
#include <stdlib.h>
#include "module.h"
#include "str/str.h"
#include "sql.h"

static MYSQL mysql;

const char sql_query_var[] = "CVM_MYSQL_QUERY";
const char sql_pwcmp_var[] = "CVM_MYSQL_PWCMP";
const char sql_postq_var[] = "CVM_MYSQL_POSTQ";

int sql_auth_init(void)
{
  const char* host;
  const char* user;
  const char* pass;
  const char* db;
  unsigned port;
  const char* unix_socket;
  const char* tmp;

  host = getenv("CVM_MYSQL_HOST");
  user = getenv("CVM_MYSQL_USER");
  pass = getenv("CVM_MYSQL_PASS");
  db = getenv("CVM_MYSQL_DB");
  tmp = getenv("CVM_MYSQL_PORT");
  port = tmp ? atoi(tmp) : 0;
  unix_socket = getenv("CVM_MYSQL_SOCKET");
  
  mysql_init(&mysql);
  if ((tmp = getenv("CVM_MYSQL_DEFAULT_FILE")) != 0)
    if (mysql_options(&mysql, MYSQL_READ_DEFAULT_FILE, tmp))
      return CVME_CONFIG;
  if ((tmp = getenv("CVM_MYSQL_DEFAULT_GROUP")) != 0)
    if (mysql_options(&mysql, MYSQL_READ_DEFAULT_GROUP, tmp))
      return CVME_CONFIG;
  if (!mysql_real_connect(&mysql, host, user, pass, db,
			  port, unix_socket, 0)) return CVME_IO;

  return 0;
}

static MYSQL_RES* result;
static MYSQL_ROW row;

int sql_post_query(const str* query)
{
  if (mysql_real_query(&mysql, query->s, query->len))
    return CVME_IO | CVME_FATAL;
  return 0;
}

int sql_auth_query(const str* query)
{
  int i;
  if ((i = sql_post_query(query)) != 0) return -i;
  result = mysql_store_result(&mysql);
  row = mysql_fetch_row(result);
  return mysql_num_rows(result);
}

const char* sql_get_field(int field)
{
  return row[field];
}

void sql_auth_stop(void)
{
  mysql_close(&mysql);
}
