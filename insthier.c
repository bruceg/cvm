#include "installer.h"
#include "conf_bin.c"

void insthier(void) {
  int bin = opendir(conf_bin);
  c(bin, 0, "cvm_unix", -1, -1, 0755);
  c(bin, 0, "testcvmclient", -1, -1, 0755);
}
