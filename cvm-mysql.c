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
#include <string.h>
#include "module.h"
#include "sql.h"
#include "str/str.h"

const unsigned cvm_credential_count = 1;
const char* cvm_credentials[1];

extern char* crypt(const char* key, const char* salt);

static const char* host;
static const char* user;
static const char* pass;
static const char* db;
static unsigned port;
static const char* unix_socket;

static MYSQL mysql;

int cvm_auth_init(void)
{
  const char* tmp;
  int i;

  host = getenv("CVM_MYSQL_HOST");
  user = getenv("CVM_MYSQL_USER");
  pass = getenv("CVM_MYSQL_PASS");
  db = getenv("CVM_MYSQL_DB");
  tmp = getenv("CVM_MYSQL_PORT");
  port = tmp ? atoi(tmp) : 0;
  unix_socket = getenv("CVM_MYSQL_SOCKET");
  if ((tmp = getenv("CVM_MYSQL_QUERY")) != 0)
    if ((i = sql_query_setup(tmp)) != 0) return i;
  
  mysql_init(&mysql);
  if (!mysql_real_connect(&mysql, 0, 0, 0, 0, 0, 0, 0)) return CVME_IO;

  return 0;
}

int cvm_authenticate(void)
{
  static str q;
  MYSQL_RES* result;
  MYSQL_ROW row;
  unsigned long* lengths;
  const char* cpw;
  int i;
  
  /* Query the database based on the custom query */
  if ((i = sql_query_build(&q)) != 0) return i;
  if (mysql_real_query(&mysql, q.s, q.len)) return CVME_IO;
  result = mysql_store_result(&mysql);

  /* If the result didn't produce a single row, fail the username */
  if (mysql_num_rows(result) != 1) return CVME_PERMFAIL;
  row = mysql_fetch_row(result);
  lengths = mysql_fetch_lengths(result);

  /* If there is no password field, fail the password */
  if (lengths[0] < 1) return CVME_PERMFAIL;
  /* If the stored encrypted password is not the right length, fail the pass */
  cpw = crypt(cvm_credentials[0], row[0]);
  if (lengths[0] != strlen(cpw)) return CVME_PERMFAIL;
  /* Finally, if the stored pass is not the same, fail the pass */
  if (memcmp(cpw, row[0], lengths[0]) != 0) return CVME_PERMFAIL;
  
  /* Credentials accepted */
  cvm_fact_username = row[1];
  cvm_fact_userid = strtol(row[2], 0, 10);
  cvm_fact_groupid = strtol(row[3], 0, 10);
  cvm_fact_realname = row[4];
  cvm_fact_directory = row[5];
  cvm_fact_shell = row[6];
  cvm_fact_groupname = 0;
  
  return 0;
}
