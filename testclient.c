#include "client.h"

int main(int argc, char** argv)
{
  int i;
  i = authenticate(argv[1], argv+2);
  if (i) return i;
  printf("uid=%d gid=%d\n", fact_userid, fact_groupid);
  return 0;
}
