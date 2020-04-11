/*
 * ProFTPD - mod_explain: open(2)
 * Copyright (c) 2020 TJ Saunders
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

#include "open.h"
#include "generic.h"
#include "path.h"

static const char *mode2text(pool *p, mode_t mode) {
  char buf[32];
  memset(buf, '\0', sizeof(buf));
  snprintf(buf, sizeof(buf)-1, "%03o", (int) mode);
  return pstrdup(p, buf);
}

static const char *oflags2text(pool *p, int flags) {
  char *text = "";

  if (flags == O_RDONLY) {
    text = pstrcat(p, text, "O_RDONLY", NULL);

  } else if (flags & O_RDWR) {
    text = pstrcat(p, text, "O_RDWR", NULL);

  } else if (flags & O_WRONLY) {
    text = pstrcat(p, text, "O_WRONLY", NULL);
  }

#ifdef O_APPEND
  if (flags & O_APPEND) {
    text = pstrcat(p, text, *text ? "|" : "", "O_APPEND", NULL);
  }
#endif

  if (flags & O_CREAT) {
    text = pstrcat(p, text, *text ? "|" : "", "O_CREAT", NULL);
  }

  if (flags & O_EXCL) {
    text = pstrcat(p, text, *text ? "|" : "", "O_EXCL", NULL);
  }

  if (flags & O_NONBLOCK) {
    text = pstrcat(p, text, *text ? "|" : "", "O_NONBLOCK", NULL);
  }

  if (flags & O_TRUNC) {
    text = pstrcat(p, text, *text ? "|" : "", "O_TRUNC", NULL);
  }

  return text;
}

static const char *get_args(pool *p, int flags, mode_t mode, const char *path) {
  const char *args;

  args = pstrcat(p, "flags = ", oflags2text(p, flags),
    ", mode = ", mode2text(p, mode),
    ", path = '", path, "'", NULL);
  return args;
}

const char *explain_open_error(pool *p, int xerrno, const char *path,
    int flags, mode_t mode, const char **args) {
  const char *explained = NULL, *syscall = "open(2)";
  int path_flags = EXPLAIN_PATH_FL_WANT_SEARCH;

  /* We need to look at the open(2) flags, to see what kind of operation
   * the caller wants to do with this path.
   */
  if (flags == O_RDONLY) {
    path_flags |= EXPLAIN_PATH_FL_WANT_READ;

  } else if ((flags & O_WRONLY) ||
             (flags & O_RDWR)) {
    path_flags |= EXPLAIN_PATH_FL_WANT_WRITE;
  }

  if (flags & O_CREAT) {
    path_flags |= EXPLAIN_PATH_FL_WANT_CREATE;

    if (flags & O_EXCL) {
      path_flags |= EXPLAIN_PATH_FL_MUST_NOT_EXIST;
    }
  }

  if (flags & O_TRUNC) {
    path_flags |= EXPLAIN_PATH_FL_WANT_MODIFY;
  }

  *args = get_args(p, flags, mode, path);

  switch (xerrno) {
    case EACCES:
    case ENOENT:
    case ELOOP:
    case ENAMETOOLONG:
    case ENOTDIR:
    case EPERM:
      explained = explain_path_error(p, xerrno, path, path_flags, S_IFREG);
      break;

    default:
      explained = explain_describe_generic(p, xerrno, syscall);
      break;
  }

  return explained;
}
