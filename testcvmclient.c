#include <stdio.h>
#include "client.h"

int main(int argc, char** argv)
{
  int i;

  if (argc < 4) {
    printf("usage: testclient cvmodule account credential [credential ...]\n");
    return 1;
  }
  
  i = authenticate(argv[1], argv+2);
  if (i) {
    printf("Authentication failed, error #%d\n", i);
    return i;
  }

  printf("user name:        %s\n"
	 "user ID:          %d\n"
	 "group ID:         %d\n"
	 "real name:        %s\n"
	 "directory:        %s\n"
	 "shell:            %s\n"
	 "group name:       %s\n"
	 "system user name: %s\n"
	 "system directory: %s\n",
	 fact_username, fact_userid, fact_groupid, fact_realname,
	 fact_directory, fact_shell, fact_groupname,
	 fact_sys_username, fact_sys_directory);
  return 0;
}
