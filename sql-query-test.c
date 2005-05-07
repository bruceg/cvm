#include <stdio.h>
#include "credentials.h"
#include "sql.h"

str cvm_credentials[CVM_CRED_MAX+1];

int main(int argc, char* argv[])
{
  static str s;
  if (argc != 4) {
    puts("usage: sql-query-test query account domain");
    return 1;
  }
  if (!sql_query_validate(argv[1])) {
    puts("sql-query-test: validation of query failed");
    return 2;
  }
  str_copys(&cvm_credentials[CVM_CRED_ACCOUNT], argv[2]);
  str_copys(&cvm_credentials[CVM_CRED_DOMAIN], argv[3]);
  if (!sql_query_build(argv[1], &s)) {
    puts("sql-query-test: query building failed");
    return 3;
  }
  puts(s.s);
  return 0;
}
