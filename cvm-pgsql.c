/* cvm/cvm-pgsql.c - PgSQL CVM
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
#include <pgsql/libpq-fe.h>
#include <stdlib.h>
#include <string.h>
#include "str/str.h"
#include "pwcmp/client.h"
#include "module.h"
#include "sql.h"

const unsigned cvm_credential_count = 1;
const char* cvm_credentials[1];

static PGconn* pg;
static const char* query;

int cvm_auth_init(void)
{
  const char* tmp;
  
  query = ((tmp = getenv("CVM_PGSQL_QUERY")) != 0) ? tmp : sql_query_default;

  if ((pg = PQconnectdb("")) == 0) return CVME_IO;
  if (PQstatus(pg) == CONNECTION_BAD) return CVME_IO;
  
  if (!pwcmp_start(getenv("CVM_PGSQL_PWCMP"))) return CVME_GENERAL;

  return 0;
}

static const char* PQgetvnull(const PGresult* result, int tup, int field)
{
  return PQgetisnull(result, tup, field) ? 0 : PQgetvalue(result, tup, field);
}

int cvm_authenticate(void)
{
  static str q;
  PGresult* result;
  const char* cpw;
  
  /* Query the database based on the custom query */  
  if (!sql_query_build(query, &q)) return CVME_GENERAL | CVME_FATAL;
  if ((result = PQexec(pg, q.s)) == 0) return CVME_IO | CVME_FATAL;
  switch (PQresultStatus(result)) {
  case PGRES_TUPLES_OK: break;
  case PGRES_COMMAND_OK: return CVME_PERMFAIL;
  default: return CVME_IO;
  }

  /* If the result didn't produce a single row, fail the username */
  if (PQntuples(result) != 1) return CVME_PERMFAIL;

  /* If there is no password field, fail the password */
  cpw = PQgetvnull(result, 0, 0);
  if (cpw == 0 || cpw[0] == 0) return CVME_PERMFAIL;
  /* Finally, if the stored pass is not the same, fail the pass */
  switch (pwcmp_check(cvm_credentials[0], cpw)) {
  case 0: break;
  case -1: return CVME_IO | CVME_FATAL;
  default: return CVME_PERMFAIL;
  }
  
  /* Credentials accepted */
  cvm_fact_username = PQgetvalue(result, 0, 1);
  cvm_fact_userid = strtol(PQgetvalue(result, 0, 2), 0, 10);
  cvm_fact_groupid = strtol(PQgetvalue(result, 0, 3), 0, 10);
  cvm_fact_realname = PQgetvnull(result, 0, 4);
  cvm_fact_directory = PQgetvalue(result, 0, 5);
  cvm_fact_shell = PQgetvnull(result, 0, 6);
  cvm_fact_groupname = 0;
  
  return 0;
}

void cvm_auth_stop(void)
{
  pwcmp_stop();
  PQfinish(pg);
}
