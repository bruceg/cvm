#include <string.h>
#include <bglibs/obuf.h>
#include <bglibs/msg.h>
#include "credentials.h"
#include "sql.h"

const char program[] = "sql-query-test";
const int msg_show_pid = 0;

str cvm_module_credentials[CVM_CRED_MAX+1];

int main(int argc, char* argv[])
{
  static str s;
  memset(cvm_module_credentials, 0, sizeof cvm_module_credentials);
  if (argc != 4)
    die3(1, "usage: ", program, " query account domain");
  if (!sql_query_validate(argv[1]))
    die1(2, "Validation of query failed");
  str_copys(&cvm_module_credentials[CVM_CRED_ACCOUNT], argv[2]);
  str_copys(&cvm_module_credentials[CVM_CRED_DOMAIN], argv[3]);
  if (!sql_query_build(argv[1], &s))
    die1(3, "Query building failed");
  obuf_putstr(&outbuf, &s);
  obuf_putsflush(&outbuf, "\n");
  return 0;
}
