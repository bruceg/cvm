#include "installer.h"
#include "conf_bin.c"

void insthier(void) {
  int bin = opendir(conf_bin);
  c(bin, 0, "cvm-benchclient",   -1, -1, 0755);
  c(bin, 0, "cvm-checkpassword", -1, -1, 0755);
  c(bin, 0, "cvm-testclient",    -1, -1, 0755);
  c(bin, 0, "cvm-unix",          -1, -1, 0755);
  c(bin, 0, "cvm-unix-local",    -1, -1, 0755);
  c(bin, 0, "cvm-unix-udp",      -1, -1, 0755);
}
