/*
 * ProFTPD - mod_explain: platform limits
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

#include "platform.h"

static long platform_sess_name_max = -1;
static long platform_sess_no_trunc = -1;
static long platform_sess_path_max = -1;

static void explain_platform_chroot_ev(const void *event_data,
    void *user_data) {
  const char *path;

  path = event_data;
  (void) user_data;

#if defined(HAVE_PATHCONF)
  platform_sess_name_max = pathconf(path, _PC_NAME_MAX);
  platform_sess_no_trunc = pathconf(path, _PC_NO_TRUNC);
  platform_sess_path_max = pathconf(path, _PC_PATH_MAX);
#endif /* HAVE_PATHCONF */
}

long explain_platform_child_max(pool *p) {
  (void) p;
  return sysconf(_SC_CHILD_MAX);
}

long explain_platform_iov_max(pool *p) {
  (void) p;
  return sysconf(_SC_IOV_MAX);
}

long explain_platform_name_max(pool *p, const char *path) {
#if defined(HAVE_PATHCONF)
  (void) p;

  if (platform_sess_name_max > 0) {
    return platform_sess_name_max;
  }

  return pathconf(path, _PC_NAME_MAX);
#else
  errno = ENOSYS;
  return -1;
#endif /* HAVE_PATHCONF */
}

long explain_platform_no_trunc(pool *p, const char *path) {
#if defined(HAVE_PATHCONF)
  (void) p;

  if (platform_sess_no_trunc > 0) {
    return platform_sess_no_trunc;
  }

  return pathconf(path, _PC_NO_TRUNC);
#else
  errno = ENOSYS;
  return -1;
#endif /* HAVE_PATHCONF */
}

long explain_platform_path_max(pool *p, const char *path) {
#if defined(HAVE_PATHCONF)
  (void) p;

  if (platform_sess_path_max > 0) {
    return platform_sess_path_max;
  }

  return pathconf(path, _PC_PATH_MAX);
#else
  errno = ENOSYS;
  return -1;
#endif /* HAVE_PATHCONF */
}

long explain_platform_open_max(pool *p) {
  (void) p;
  return sysconf(_SC_OPEN_MAX);
}

void explain_platform_init(pool *p) {
  (void) p;

  /* Register a listener for the chroot event, so that we can look up
   * system limits, based on paths, BEFORE the chroot(2) happens.
   */
  pr_event_register(&explain_module, "core.chroot", explain_platform_chroot_ev,
    NULL);
}

