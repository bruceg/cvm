#include <stdio.h>
#include "sql.h"

const char* cvm_account_name;
const char* cvm_account_domain;

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
  cvm_account_name = argv[2];
  cvm_account_domain = argv[3];
  if (!sql_query_build(argv[1], &s)) {
    puts("sql-query-test: query building failed");
    return 3;
  }
  puts(s.s);
  return 0;
}
