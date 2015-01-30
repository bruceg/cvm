/* cvm/cvm-mysql.c - MySQL CVM
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
#include <mysql/errmsg.h>
#include <mysql/mysql.h>
#include <stdlib.h>
#include <unistd.h>
#include <bglibs/str.h>
#include "module.h"
#include "sql.h"

const char program[] = "cvm-mysql";

const char sql_query_var[] = "CVM_MYSQL_QUERY";
const char sql_pwcmp_var[] = "CVM_MYSQL_PWCMP";
const char sql_postq_var[] = "CVM_MYSQL_POSTQ";

static MYSQL mysql;

static const char* host;
static const char* user;
static const char* pass;
static const char* db;
static unsigned port;
static const char* unix_socket;

static int do_connect(void)
{
  if (!mysql_real_connect(&mysql, host, user, pass, db,
			  port, unix_socket, 0)) return CVME_IO;
  return 0;
}

int sql_auth_init(void)
{
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
  return do_connect();
}

int sql_post_query(const str* query)
{
  int i;
  if (mysql_real_query(&mysql, query->s, query->len) == 0) return 0;
  if (mysql_errno(&mysql) != CR_SERVER_LOST) return CVME_IO | CVME_FATAL;
  mysql_close(&mysql);
  if ((i = do_connect()) != 0) return i;
  if (mysql_real_query(&mysql, query->s, query->len) == 0) return 0;
  return CVME_IO | CVME_FATAL;
}

static MYSQL_ROW row;

int sql_auth_query(const str* query)
{
  int i;
  static MYSQL_RES* result = 0;
  if ((i = sql_post_query(query)) != 0) return -i;
  if (result != 0) mysql_free_result(result);
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
