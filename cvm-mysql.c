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
#include "str/str.h"
#include "pwcmp/client.h"
#include "module.h"
#include "sql.h"

const unsigned cvm_credential_count = 1;
const char* cvm_credentials[1];

static MYSQL mysql;
static const char* query;

int cvm_auth_init(void)
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
  query = ((tmp = getenv("CVM_MYSQL_QUERY")) != 0) ? tmp : sql_query_default;
  
  mysql_init(&mysql);
  if (!mysql_real_connect(&mysql, host, user, pass, db,
			  port, unix_socket, 0)) return CVME_IO;

  if (!pwcmp_start(getenv("CVM_MYSQL_PWCMP"))) return CVME_GENERAL;

  return 0;
}

int cvm_authenticate(void)
{
  static str q;
  MYSQL_RES* result;
  MYSQL_ROW row;
  unsigned long* lengths;
  
  /* Query the database based on the custom query */
  if (!sql_query_build(query, &q)) return CVME_GENERAL | CVME_FATAL;
  if (mysql_real_query(&mysql, q.s, q.len)) return CVME_IO | CVME_FATAL;
  result = mysql_store_result(&mysql);

  /* If the result didn't produce a single row, fail the username */
  if (mysql_num_rows(result) != 1) return CVME_PERMFAIL;
  row = mysql_fetch_row(result);
  lengths = mysql_fetch_lengths(result);

  /* If there is no password field, fail the password */
  if (lengths[0] < 1) return CVME_PERMFAIL;
  /* Finally, if the stored pass is not the same, fail the pass */
  switch (pwcmp_check(cvm_credentials[0], row[0])) {
  case 0: break;
  case -1: return CVME_IO | CVME_FATAL;
  default: return CVME_PERMFAIL;
  }
  
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

void cvm_auth_stop(void)
{
  pwcmp_stop();
  mysql_close(&mysql);
}
