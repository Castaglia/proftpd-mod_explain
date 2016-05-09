/*
 * ProFTPD - mod_explain: chroot(2)
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

#include "chroot.h"
#include "generic.h"
#include "path.h"

#if defined(ENOMEM)
static const char *describe_kernel_enomem(pool *p, const char *syscall) {
  return pstrcat(p, "there was not enough kernel memory available for ",
    syscall, " to succeed", NULL);
}
#endif /* ENOMEM */

static const char *get_args(pool *p, const char *path) {
  const char *args;

  args = pstrcat(p, "path = '", path, "'", NULL);
  return args;
}

const char *explain_chroot_error(pool *p, int xerrno, const char *path,
    const char **args) {
  const char *explained = NULL, *syscall = "chroot(2)";
  int path_flags = EXPLAIN_PATH_FL_WANT_SEARCH|EXPLAIN_PATH_FL_MUST_HAVE_MODE;

  *args = get_args(p, path);

  switch (xerrno) {
    case EACCES:
    case ENOENT:
    case ELOOP:
    case ENAMETOOLONG:
    case ENOTDIR:
    case EPERM:
      explained = explain_path_error(p, xerrno, path, path_flags, S_IFDIR);
      break;

#if defined(ENOMEM)
    case ENOMEM:
      explained = describe_kernel_enomem(p, syscall);
      break;
#endif /* ENOMEM */

    default:
      explained = explain_describe_generic(p, xerrno, syscall);
      break;
  }

  return explained;
}
