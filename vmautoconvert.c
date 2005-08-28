/* vmautoconvert.c - Automatically convert passwords for vmailmgr.
 * Copyright (C) 2005  Bruce Guenter <bruceg@em.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <sysdeps.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cdb/make.h>
#include <iobuf/iobuf.h>
#include <path/path.h>
#include <str/str.h>
#include <vmailmgr/vpwentry.h>
#include <uint32.h>

#include "module.h"
#include "qmail.h"
#include "cvm-vmailmgr.h"

static int read_start(ibuf* in, uint32* end)
{
  unsigned char buf[2048];
  if (!ibuf_read(in, buf, sizeof buf))
    return 0;
  *end = uint32_get_lsb(buf);
  return 1;
}

static int read_cdb_pair(ibuf* in, str* key, str* data)
{
  unsigned char buf[8];
  uint32 keylen;
  uint32 datalen;
  if (!ibuf_read(in, buf, sizeof buf))
    return 0;
  keylen = uint32_get_lsb(buf);
  datalen = uint32_get_lsb(buf+4);
  if (!str_ready(key, keylen)
      || !str_ready(data, datalen)
      || !ibuf_read(in, key->s, keylen)
      || !ibuf_read(in, data->s, datalen))
    return 0;
  key->s[key->len = keylen] = 0;
  data->s[data->len = datalen] = 0;
  return 1;
}

static str tmppwfile;
static str key;
static str data;

static int convert_data(void)
{
  struct vpwentry vpw;
  int status = 1;
  memset(&vpw, 0, sizeof vpw);
  if (!vpwentry_import(&vpw, &virtuser, &data))
    return 0;
  status = str_copyb(&vpw.pass, "$0$", 3)
    && str_cat(&vpw.pass, &cvm_module_credentials[CVM_CRED_PASSWORD])
    && vpwentry_export(&vpw, &data);
  vpwentry_free(&vpw);
  return status;
}

int vmailmgr_autoconvert(void)
{
  int writefd = -1;
  ibuf reader;
  struct cdb_make writer;
  int error = 0;
  int readall = 0;
  int writerr = 0;
  if ((writefd = path_mktemp(pwfile, &tmppwfile)) != -1) {

    if (cdb_make_start(&writer, writefd) != 0)
      error = CVME_IO | CVME_FATAL;
    else {

      if (ibuf_open(&reader, pwfile, 0)) {

	uint32 end;
	struct stat st;
	if (fstat(reader.io.fd, &st) == 0
	    && fchmod(writefd, st.st_mode) == 0
	    && fchown(writefd, st.st_uid, st.st_gid) == 0
	    && read_start(&reader, &end)) {
	  while (ibuf_tell(&reader) < end) {
	    if (!read_cdb_pair(&reader, &key, &data))
	      break;
	    if (str_diff(&key, &virtuser) == 0)
	      if (!convert_data()) {
		writerr = 1;
		break;
	      }
	    if (cdb_make_add(&writer, key.s, key.len, data.s, data.len) != 0) {
	      writerr = 1;
	      break;
	    }
	  }
	  readall = ibuf_tell(&reader) == end;
	}
	ibuf_close(&reader);
      }
      if (cdb_make_finish(&writer) != 0)
	error |= CVME_FATAL;
      else
	if (readall && !writerr)
	  rename(tmppwfile.s, pwfile);
    }
    close(writefd);
    unlink(tmppwfile.s);
  }
  return error;
}
