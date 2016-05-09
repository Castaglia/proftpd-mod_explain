/*
 * ProFTPD - mod_explain: generically applicable errno values
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

#include "generic.h"

/* The following code describes various "generic" errors.  Many system calls
 * will have more specific/relevant descriptions for what these error values
 * mean in the context of those calls; the following descriptions are for
 * cases where the syscall-specific code does NOT have a more relevant
 * description.
 */

#if defined(EAGAIN)
static const char *describe_eagain(pool *p, const char *syscall) {
  return pstrcat(p, "the ", syscall,
    " system call was waiting to finish but was told not to wait", NULL);
}
#endif /* EAGAIN */

#if defined(EMFILE)
static const char *describe_emfile(pool *p, const char *syscall) {
  /* TODO: Determine a way to report just what the system limit on open files
   * is, for better reporting.  Maybe this can be done at process init,
   * in case we are later chrooted?  I.e. call sysconf(_SC_OPEN_MAX) early,
   * and stash/use that number here.
   */
  (void) syscall;

  return pstrdup(p,
    "the process already has the maximum number of file descriptors open");
}
#endif /* EMFILE */

#if defined(ENFILE)
static const char *describe_enfile(pool *p, const char *syscall) {
  /* TODO: Determine a way to report just what the system limit on open files
   * is, for better reporting.  Maybe this can be done at process init,
   * in case we are later chrooted?
   */
  (void) syscall;

  return pstrdup(p,
    "the system limit on the total number of open files has been reached");
}
#endif /* ENFILE */

#if defined(EINTR)
static const char *describe_eintr(pool *p, const char *syscall) {
  return pstrcat(p, "the ", syscall,
    " system call was interrupted by a signal before it could finish", NULL);
}
#endif /* EINTR */

#if defined(EFAULT)
static const char *describe_efault(pool *p, const char *syscall) {
  return pstrcat(p, "one of the ", syscall,
    " parameters is null, uninitialized, or points to invalid/unreachable "
    "memory", NULL);
}
#endif /* EFAULT */

#if defined(EIO)
static const char *describe_eio(pool *p, const char *syscall) {
  (void) syscall;
  return pstrdup(p,
    "a low-level filesystem I/O error occurred, probably in hardware");
}
#endif /* EIO */

#if defined(ENOMEM)
static const char *describe_enomem(pool *p, const char *syscall) {
  return pstrcat(p, "there was not enough user-space memory available for ",
    syscall, " to succeed", NULL);
}
#endif /* ENOMEM */

#if defined(EPERM)
static const char *describe_eperm(pool *p, const char *syscall) {
  return pstrcat(p,
    "the process does not have the appropriate privileges to use the ",
    syscall, " system call", NULL);
}
#endif /* EPERM */

static const char *describe_generic(pool *p, const char *syscall) {
  return pstrcat(p, "the entropic gremlins have frobnicated the ", syscall,
    " system call", NULL);
}

const char *explain_describe_generic(pool *p, int xerrno, const char *syscall) {
  const char *explained = NULL;

  switch (xerrno) {
    case 0:
      /* Not an error. */
      break;

#if defined(EAGAIN)
    case EAGAIN:
# if defined(EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK:
# endif /* EWOULDBLOCK != EAGAIN */
      explained = describe_eagain(p, syscall);
      break;
#endif /* EAGAIN */

#if defined(EMFILE)
    case EMFILE:
      explained = describe_emfile(p, syscall);
      break;
#endif /* EMFILE */

#if defined(ENFILE)
    case ENFILE:
      explained = describe_enfile(p, syscall);
      break;
#endif /* ENFILE */

#if defined(EINTR)
    case EINTR:
      explained = describe_eintr(p, syscall);
      break;
#endif /* EINTR */

#if defined(EFAULT)
    case EFAULT:
      explained = describe_efault(p, syscall);
      break;
#endif /* EFAULT */

#if defined(EIO)
    case EIO:
      explained = describe_eio(p, syscall);
      break;
#endif /* EIO */

#if defined(ENOMEM)
    case ENOMEM:
      explained = describe_enomem(p, syscall);
      break;
#endif /* ENOMEM */

#if defined(EPERM)
    case EPERM:
      explained = describe_eperm(p, syscall);
      break;
#endif /* EPERM */

    default:
      explained = describe_generic(p, syscall);
      break;
  }

  return explained;
}
