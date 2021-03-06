/* cvm/sql-auth.c - Generic SQL authentication layer
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
#include <stdlib.h>
#include <string.h>
#include <bglibs/str.h>
#include <pwcmp/client.h>
#include "module.h"
#include "sql.h"

static const char* query;
static const char* postq;

int cvm_module_init(void)
{
  int result;

  if ((query = getenv(sql_query_var)) == 0) return CVME_CONFIG;
  if (!sql_query_validate(query)) return CVME_CONFIG;

  if ((postq = getenv(sql_postq_var)) != 0)
    if (!sql_query_validate(postq)) return CVME_CONFIG;
  
  if ((result = sql_auth_init()) != 0) return result;
  
  if (!pwcmp_start(getenv(sql_pwcmp_var))) return CVME_GENERAL;

  return 0;
}

static str q;

int cvm_module_lookup(void)
{
  int i;

  /* Query the database based on the custom query */  
  if (!sql_query_build(query, &q)) return CVME_GENERAL | CVME_FATAL;
  if ((i = sql_auth_query(&q)) < 0) return -i;

  /* If the result didn't produce a single row, fail the username */
  return (i == 1) ? 0 : CVME_PERMFAIL;
}

int cvm_module_authenticate(void)
{
  const char* cpw;

  CVM_CRED_REQUIRED(PASSWORD);
  
  /* If there is no password field, fail the password */
  cpw = sql_get_field(0);
  if (cpw == 0 || cpw[0] == 0) return CVME_PERMFAIL;

  /* Finally, if the stored pass is not the same, fail the pass */
  switch (pwcmp_check(cvm_module_credentials[CVM_CRED_PASSWORD].s, cpw)) {
  case 0: return 0;
  case -1: return CVME_IO | CVME_FATAL;
  default: return CVME_PERMFAIL;
  }
}

int cvm_module_results(void)
{
  int i;

  if (postq) {
    if (!sql_query_build(postq, &q)) return CVME_GENERAL | CVME_FATAL;
    if ((i = sql_post_query(&q)) != 0) return i;
  }
  
  /* Credentials accepted */
  cvm_fact_username = sql_get_field(1);
  cvm_fact_userid = strtol(sql_get_field(2), 0, 10);
  cvm_fact_groupid = strtol(sql_get_field(3), 0, 10);
  cvm_fact_directory = sql_get_field(4);
  cvm_fact_realname = sql_get_field(5);
  cvm_fact_shell = sql_get_field(6);
  cvm_fact_groupname = sql_get_field(7);
  cvm_fact_domain = sql_get_field(8);
  cvm_fact_sys_username = sql_get_field(9);
  cvm_fact_sys_directory = sql_get_field(10);
  cvm_fact_mailbox = sql_get_field(11);
  
  return 0;
}

void cvm_module_stop(void)
{
  pwcmp_stop();
  sql_auth_stop();
}
