/* cvm-sqlite.c
** cvm module for sqlite-backed credential database
** wcm, 2007.12.28 - 2007.12.31
** ===
*/

/* standard headers: */
#include <stdlib.h>

/* bglib headers: */
#include <str/str.h>

/* sqlite: */
#include <sqlite3.h>

/* cvm interfaces: */
#include <cvm/module.h>

/* cvm-sql interface: */
#include "sql.h"

const char program[] = "cvm-sqlite";

const char sql_query_var[] = "CVM_SQLITE_QUERY";
const char sql_pwcmp_var[] = "CVM_SQLITE_PWCMP";
const char sql_postq_var[] = "CVM_SQLITE_POSTQ";

/* struct for one row of query result: */
struct sql_row {
  int     nfields;
  char  **data;
};

/* sqlite3 query callback interface (called for each row in query result): */
static int
my_callback(void *callback_arg, int nfields, char **data, char **field_names);

/* variables in scope of this file: */
static const char *dbfile;
static sqlite3 *db;
static int result_rows;
static struct sql_row row;



/* implementation: */

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
  row.nfields = nfields;
  row.data = data;
  ++result_rows;
  return 0;
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
