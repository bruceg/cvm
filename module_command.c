#include <unistd.h>
#include "module.h"

static int read_input(void)
{
  size_t rd;

  inbuflen = 0;
  for (inbuflen = 0; inbuflen <= BUFSIZE; inbuflen += rd) {
    if ((rd = read(0, inbuffer+inbuflen, BUFSIZE-inbuflen)) == (unsigned)-1)
      return CVME_IO;
    if (rd == 0) break;
  }
  return 0;
}

static int write_output(void)
{
  size_t wr;
  char* ptr;
  
  for (ptr = outbuffer; outbuflen > 0; outbuflen -= wr, ptr += wr) {
    if ((wr = write(1, ptr, outbuflen)) == (unsigned)-1 || wr == 0)
      return CVME_IO;
  }
  return 0;
}

int main(void)
{
  int code;
  if ((code = read_input()) != 0) return code;
  if ((code = handle_request()) != 0) return code;
  fact_end(code);
  if ((code = write_output()) != 0) return code;
  return 0;
}
