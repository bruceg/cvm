#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <installer.h>
#include "conf_bin.c"
#include "conf_include.c"
#include "conf_lib.c"

void insthier(void) {
  struct stat st;
  const int do_mysql = stat("cvm-mysql", &st) != -1;
  const int do_pgsql = stat("cvm-pgsql", &st) != -1;
  const int do_vchkpw = stat("cvm-vchkpw", &st) != -1;
  int dir;

  dir = opendir(conf_bin);
  c(dir, "cvm-benchclient",     -1, -1, 0755);
  c(dir, "cvm-chain",           -1, -1, 0755);
  c(dir, "cvm-checkpassword",   -1, -1, 0755);
  if (do_mysql) {
    c(dir, "cvm-mysql",           -1, -1, 0755);
    c(dir, "cvm-mysql-local",     -1, -1, 0755);
    c(dir, "cvm-mysql-udp",       -1, -1, 0755);
  }
  if (do_pgsql) {
    c(dir, "cvm-pgsql",           -1, -1, 0755);
    c(dir, "cvm-pgsql-local",     -1, -1, 0755);
    c(dir, "cvm-pgsql-udp",       -1, -1, 0755);
  }
  c(dir, "cvm-pwfile",          -1, -1, 0755);
  c(dir, "cvm-qmail",           -1, -1, 0755);
  c(dir, "cvm-testclient",      -1, -1, 0755);
  c(dir, "cvm-v1benchclient",   -1, -1, 0755);
  c(dir, "cvm-v1checkpassword", -1, -1, 0755);
  c(dir, "cvm-v1testclient",    -1, -1, 0755);
  c(dir, "cvm-vmailmgr",        -1, -1, 0755);
  c(dir, "cvm-vmailmgr-local",  -1, -1, 0755);
  c(dir, "cvm-vmailmgr-udp",    -1, -1, 0755);
  c(dir, "cvm-unix",            -1, -1, 0755);
  if (do_vchkpw) {
    c(dir, "cvm-vchkpw",        -1, -1, 0755);
  }

  dir = opendir(conf_include);
  s(dir, "cvm-sasl.h",          "cvm/sasl.h");
  dir = d(dir,  "cvm",          -1, -1, 0755);
  s(dir, "client.h",            "v1client.h");
  c(dir, "credentials.h",       -1, -1, 0644);
  c(dir, "errors.h",            -1, -1, 0644);
  c(dir, "facts.h",             -1, -1, 0644);
  c(dir, "module.h",            -1, -1, 0644);
  c(dir, "protocol.h",          -1, -1, 0644);
  c(dir, "sasl.h",              -1, -1, 0644);
  c(dir, "v1client.h",          -1, -1, 0644);
  c(dir, "v2client.h",          -1, -1, 0644);

  dir = opendir(conf_lib);
  s(dir,  "libcvm-client.a",    "libcvm-v1client.a");
  cf(dir, "libcvm-command.a",   -1, -1, 0644, "command.a");
  cf(dir, "libcvm-local.a",     -1, -1, 0644, "local.a");
  cf(dir, "libcvm-module.a",    -1, -1, 0644, "module.a");
  cf(dir, "libcvm-sasl.a",      -1, -1, 0644, "sasl.a");
  cf(dir, "libcvm-udp.a",       -1, -1, 0644, "udp.a");
  cf(dir, "libcvm-v1client.a",  -1, -1, 0644, "v1client.a");
  cf(dir, "libcvm-v2client.a",  -1, -1, 0644, "v2client.a");
}
