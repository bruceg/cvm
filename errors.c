/* cvm/errors.c - Error strings
 * Copyright (C) 2010  Bruce Guenter <bruce@untroubled.org>
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
#include "errors.h"

static const char na[] = "Unknown error";

const char* const cvm_errlist[] = {
  "No error",			/* 0 */
  "General error",		/* 1 CVME_GENERAL */
  "Invalid data from client",	/* 2 CVME_BAD_CLIDATA */
  "Invalid data from module",	/* 3 CVME_BAD_MODDATA */
  "Input/Output error",		/* 4 CVME_IO */
  "Fact missing in data from client", /* 5 CVME_NOFACT */
  "Configuration error",	/* 6 CVME_CONFIG */
  "Credential missing in data from module", /* 7 CVME_NOCRED */
  na,
  na,
  na,na,na,na,na,na,na,na,na,na, /* 10-19 */
  na,na,na,na,na,na,na,na,na,na, /* 20-29 */
  na,na,na,na,na,na,na,na,na,na, /* 30-39 */
  na,na,na,na,na,na,na,na,na,na, /* 40-49 */
  na,na,na,na,na,na,na,na,na,na, /* 50-59 */
  na,na,na,na,na,na,na,na,na,na, /* 60-69 */
  na,na,na,na,na,na,na,na,na,na, /* 70-79 */
  na,na,na,na,na,na,na,na,na,na, /* 80-89 */
  na,na,na,na,na,na,na,na,na,na, /* 90-99 */
  "Credentials rejected",	/* 100 CVME_PERMFAIL */
};

const int cvm_nerr = 101;
