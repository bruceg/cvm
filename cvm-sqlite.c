/* cvm/cvm-sqlite.c - SQLite CVM
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
 *
 * Placed into the public domain by Wayne Marshall <wcm@guinix.com>
 */

#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#include <bglibs/str.h>
#include <cvm/module.h>
#include <bglibs/msg.h>

#include "sql.h"

const char program[] = "cvm-sqlite";

const char sql_query_var[] = "CVM_SQLITE_QUERY";
const char sql_pwcmp_var[] = "CVM_SQLITE_PWCMP";
const char sql_postq_var[] = "CVM_SQLITE_POSTQ";

#define MAX_NFIELDS 12

/* struct for one row of query result: */
struct sql_row {
  int nfields;
  const char* data[MAX_NFIELDS];
};

/* variables in scope of this file: */
static const char *dbfile;
static sqlite3 *db;
static int result_rows;
static struct sql_row row;

/*
** sqlite3 query callback interface
**
**   * called for each row in query result
**   * cvm design assumes/requires only one row returned per query
**   * return of more than one row results in: error #100 (Credentials rejected)
**
*/
static int my_callback(void *callback_arg, int nfields, char **data, char **field_names)
{
  int i;
  row.nfields = nfields;
  for (i = 0; i < MAX_NFIELDS && i < nfields; ++i) {
    if (row.data[i] != NULL)
      free((char*)row.data[i]);
    if (data[i] != NULL)
      if ((data[i] = strdup(data[i])) == NULL)
	return 1;
    row.data[i] = data[i];
  }
  ++result_rows;
  return 0;
  (void)callback_arg;
  (void)field_names;
}


int sql_auth_init(void)
{
  int  err = 0;

  dbfile = getenv("CVM_SQLITE_DB");
  if(dbfile == NULL)
      return CVME_CONFIG;

  err = sqlite3_open(dbfile, &db);
  if(err != SQLITE_OK)
      return CVME_IO;

  err = sqlite3_busy_timeout(db, 1000);
  if(err != SQLITE_OK)
      return CVME_FATAL;

  result_rows = 0;
  return 0;
}


int sql_auth_query(const str *query)
{
  int  err = 0;

  result_rows = 0;
  err = sqlite3_exec(db, query->s, &my_callback, NULL, NULL);
  if(err != SQLITE_OK) return -(CVME_IO | CVME_FATAL);

  return result_rows;
}


/* XXX, not sure how this might be used: */
int sql_post_query(const str *query)
{
  return 0;
  (void)query;
}


/*
** NOTE:
**
**   cvm design assumes/requires credential query to select fields
**   in an exact specific order
**
**   that is, field names don't matter
**   what matters is field order in the result set
**
**   btw, defining CVM_SQLITE_QUERY in the module environment is *required*
**   that is, no default query is provided
**
*/
const char * sql_get_field(int n)
{
  return (n < row.nfields ? row.data[n] : NULL);
}


void sql_auth_stop(void)
{
  sqlite3_close(db);
  return;
}


/* cvm-sqlite.c: EOF */
