#include "module.h"

int main(void)
{
  read_input();
  authenticate();
  fact_str(FACT_USERNAME, fact_username);
  fact_uint(FACT_USERID, fact_userid);
  fact_uint(FACT_GROUPID, fact_groupid);
  if (fact_realname) fact_str(FACT_REALNAME, fact_realname);
  fact_str(FACT_DIRECTORY, fact_directory);
  fact_str(FACT_SHELL, fact_shell);
  if (fact_groupname) fact_str(FACT_GROUPNAME, fact_groupname);
  if (fact_sys_username) fact_str(FACT_SYS_USERNAME, fact_sys_username);
  if (fact_sys_directory) fact_str(FACT_SYS_DIRECTORY, fact_sys_directory);
  fact_end();
  return 0;
}
