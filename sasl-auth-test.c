#include <bglibs/iobuf.h>
#include <bglibs/msg.h>
#include <bglibs/str.h>
#include "sasl.h"

const char program[] = "sasl-auth-test";
const int msg_show_pid = 0;

struct sasl_auth sa = {
  .prefix = "+ ",
  .in = &inbuf,
  .out = &outbuf,
};

int main(int argc, char* argv[])
{
  static str s;
  int i;
  int j;
  const char* msg;
  if (argc != 2)
    die3(1, "usage: ", program, " string");
  if (!sasl_auth_init(&sa))
    die1(1, "sasl_auth_init failed");
  if (!sasl_auth_caps(&s))
    die1(1, "sasl_auth_cap failed");
  msg1(s.s);
  str_copys(&s, argv[1]);
  if ((i = sasl_auth1(&sa, &s)) != 0) {
    j = i;
    msg = sasl_auth_msg(&j);
    dief(1, "{sasl_auth1 failed: }d{\n  }d{ }s", i, j, msg);
  }
  return 0;
  (void)argc;
}
