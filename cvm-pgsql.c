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
#include "str/str.h"
#include "module.h"
#include "sql.h"

const char sql_query_var[] = "CVM_PGSQL_QUERY";
const char sql_pwcmp_var[] = "CVM_PGSQL_PWCMP";

static PGconn* pg;

int sql_auth_init(void)
{
  if ((pg = PQconnectdb("")) == 0) return CVME_IO;
  if (PQstatus(pg) == CONNECTION_BAD) return CVME_IO;
  return 0;
}

static PGresult* result;

const char* sql_get_field(int field)
{
  return PQgetisnull(result, 0, field) ? 0 : PQgetvalue(result, 0, field);
}

int sql_auth_query(const str* query)
{
  if ((result = PQexec(pg, query->s)) == 0) return -(CVME_IO | CVME_FATAL);
  switch (PQresultStatus(result)) {
  case PGRES_TUPLES_OK: return PQntuples(result);
  case PGRES_COMMAND_OK: return -CVME_PERMFAIL;
  default: return -CVME_IO;
  }
}

void sql_auth_stop(void)
{
  PQfinish(pg);
}
