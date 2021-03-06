/* cvm/facts.c - Predefined credential facts
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
#include "facts.h"

const char* cvm_fact_username = 0;
unsigned long cvm_fact_userid = 0;
unsigned long cvm_fact_groupid = 0;
const char* cvm_fact_realname = 0;
const char* cvm_fact_directory = 0;
const char* cvm_fact_shell = 0;
const char* cvm_fact_groupname = 0;
const char* cvm_fact_sys_username = 0;
const char* cvm_fact_sys_directory = 0;
const char* cvm_fact_domain = 0;
const char* cvm_fact_mailbox = 0;
