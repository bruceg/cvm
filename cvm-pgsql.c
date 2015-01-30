/* cvm/cvm-pgsql.c - PgSQL CVM
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
#include <libpq-fe.h>
#include <stdlib.h>
#include <bglibs/str.h>
#include "module.h"
#include "sql.h"

const char program[] = "cvm-pgsql";

const char sql_query_var[] = "CVM_PGSQL_QUERY";
const char sql_pwcmp_var[] = "CVM_PGSQL_PWCMP";
const char sql_postq_var[] = "CVM_PGSQL_POSTQ";

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

static int pgsql_query(const str* query)
{
  if (result) PQclear(result);
  if ((result = PQexec(pg, query->s)) != 0) return 1;
  if (PQstatus(pg) != CONNECTION_BAD) return 0;
  PQreset(pg);
  if ((result = PQexec(pg, query->s)) != 0) return 1;
  return 0;
}

int sql_post_query(const str* query)
{
  if (!pgsql_query(query)) return CVME_IO | CVME_FATAL;
  switch (PQresultStatus(result)) {
  case PGRES_TUPLES_OK:
  case PGRES_COMMAND_OK:
    return 0;
  default: return CVME_IO;
  }
}

int sql_auth_query(const str* query)
{
  if (!pgsql_query(query)) return -(CVME_IO | CVME_FATAL);
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
