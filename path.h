/*
 * ProFTPD - mod_explain: path_resolution(2)
 * Copyright (c) 2016 TJ Saunders
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
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 */

#ifndef MOD_EXPLAIN_PATH_H
#define MOD_EXPLAIN_PATH_H

#include "mod_explain.h"

const char *explain_path_error(pool *p, int xerrno, const char *path,
  int flags, mode_t mode);

/* Provide reasons why we want to use this path. */
#define EXPLAIN_PATH_FL_WANT_READ		0x0001
#define EXPLAIN_PATH_FL_WANT_WRITE		0x0002
#define EXPLAIN_PATH_FL_WANT_SEARCH		0x0004
#define EXPLAIN_PATH_FL_WANT_CREATE		0x0008
#define EXPLAIN_PATH_FL_WANT_MODIFY		0x0010
#define EXPLAIN_PATH_FL_WANT_UNLINK		0x0020
#define EXPLAIN_PATH_FL_MUST_EXIST		0x0040
#define EXPLAIN_PATH_FL_MUST_NOT_EXIST		0x0080
#define EXPLAIN_PATH_FL_MUST_HAVE_MODE		0x0100

#endif /* MOD_EXPLAIN_PATH_H */
