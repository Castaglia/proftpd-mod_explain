/*
 * ProFTPD - mod_explain: lstat(2)
 * Copyright (c) 2017 TJ Saunders
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

#include "lstat.h"
#include "generic.h"
#include "path.h"

static const char *get_args(pool *p, const char *path) {
  const char *args;

  args = pstrcat(p, "path = '", path, "'", NULL);
  return args;
}

const char *explain_lstat_error(pool *p, int xerrno, const char *path,
    struct stat *st, const char **args) {
  const char *explained = NULL, *syscall = "lstat(2)";
  int path_flags = EXPLAIN_PATH_FL_WANT_SEARCH|EXPLAIN_PATH_FL_MUST_HAVE_MODE;

  *args = get_args(p, path);

  switch (xerrno) {
    case EACCES:
    case ENOENT:
    case ELOOP:
    case ENAMETOOLONG:
    case EPERM:
      explained = explain_path_error(p, xerrno, path, path_flags, S_IFREG);
      break;

    default:
      explained = explain_describe_generic(p, xerrno, syscall);
      break;
  }

  return explained;
}
