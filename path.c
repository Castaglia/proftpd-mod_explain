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

#include "path.h"
#include "platform.h"

static const char *ul2s(pool *p, unsigned long l) {
  char buf[1024];
  memset(buf, '\0', sizeof(buf));
  snprintf(buf, sizeof(buf)-1, "%lu", l);
  return pstrdup(p, buf);
}

static array_header *path_split(pool *p, const char *path) {
  char buf[PR_TUNABLE_PATH_MAX+1], *full_path, *ptr;
  size_t full_pathlen;
  array_header *components;

  pr_fs_virtual_path(path, buf, sizeof(buf)-1);
  full_path = buf;
  full_pathlen = strlen(full_path);

  /* If the path ENDS in '/', append '.' to it. */
  if (full_path[full_pathlen-1] == '/' &&
      full_pathlen < (sizeof(buf)-1)) {
    full_path[full_pathlen] = '.';
  }

  components = make_array(p, 1, sizeof(char *));

  /* The first component is ALWAYS '/'. */
  *((char **) push_array(components)) = pstrdup(p, "/");

  if (full_pathlen == 1) {
    return components;
  }

  ptr = full_path + 1;
  while (ptr != NULL) {
    char *ptr2;

    pr_signals_handle();

    ptr2 = strchr(ptr, '/');
    if (ptr2 == NULL) {
      /* Last component in the path. */
      *((char **) push_array(components)) = pstrdup(p, ptr);
      ptr = NULL;
      continue;
    }

    *((char **) push_array(components)) = pstrndup(p, ptr, ptr2 - ptr);
    ptr = ptr2 + 1;
  }

  return components;
}

static const char *describe_enametoolong_name(pool *p, const char *name,
    size_t name_len, unsigned long name_max) {
  return pstrcat(p, "path component '", name,
    "' exceeds the system maximum name length (",
    ul2s(p, (unsigned long) name_len), " > max ", ul2s(p, name_max), ")", NULL);
}

static const char *describe_enametoolong_path(pool *p, const char *path,
    size_t path_len, unsigned long path_max) {
  return pstrcat(p, "'", path, "' exceeds the system maximum path length (",
    ul2s(p, (unsigned long) path_len), " > max ", ul2s(p, path_max), ")", NULL);
}

static const char *describe_enoent_path(pool *p, const char *path) {
  return pstrcat(p, "directory '", path, "' does not exist", NULL);
}

const char *explain_path_error(pool *p, int xerrno, const char *full_path,
    int flags, mode_t mode) {
  register unsigned int i;
  array_header *components = NULL;
  const char *explained = NULL, *path = NULL;
  unsigned long name_max, no_trunc;

  if (p == NULL ||
      full_path == NULL) {
    errno = EINVAL;
    return NULL;
  }

  /* Try to get some of the easy cases out of the way first. */

  if (xerrno == ENAMETOOLONG) {
    unsigned long path_max;

    path_max = explain_platform_path_max(p, full_path);
    if (path_max > 0) {
      size_t path_len;

      path_len = strlen(full_path);
      if (path_len > path_max) {
        explained = describe_enametoolong_path(p, full_path, path_len,
          path_max);
        return explained;
      }
    }
  }

  /* Now we need to walk the path.  Whee.
   *
   * To do this, we split the path into an array, one element per component.
   * This SHOULD make it easier to progressively construct the longer and
   * longer paths, leading up to the final component/full path, for checking
   * each fully qualified component along the way.
   */

  components = path_split(p, full_path);

  name_max = explain_platform_name_max(p, full_path);
  no_trunc = explain_platform_no_trunc(p, full_path);

  for (i = 0; i < components->nelts; i++) {
    const char **elts, *component;
    int final_component = FALSE;

    pr_signals_handle();

    elts = components->elts;
    component = elts[i];

    if (xerrno == ENAMETOOLONG) {
      size_t component_len;

      component_len = strlen(component);

      /* Component names can be too long only if the filesystem on which
       * the path resides does not silently truncate long names.
       */
      if (component_len > name_max &&
          no_trunc == 1) {
        explained = describe_enametoolong_name(p, component, component_len,
          name_max);
        return explained;
      }
    }

    if (path == NULL) {
      path = component;

    } else {
      path = pdircat(p, path, component, NULL);
    }

    /* The following are the checks on the non-final components.  The final
     * component has different constraints.
     */

    final_component = (i == (components->nelts-1));

    if (final_component == FALSE) {
      int res;
      struct stat st;

      res = pr_fsio_lstat(path, &st);
      if (res < 0) {
        if (errno == ENOENT) {
          explained = describe_enoent_path(p, path);
        }

        break; 
      }

      /* XXX What if this is a symlink? */

      if (!S_ISDIR(st.st_mode)) {
        /* Explains ENOTDIR */
        explained = pstrcat(p, "path '", path,
          "' does not refer to a directory", NULL);
        break;
      }

      /* if WANT_SEARCH and no search, EACCES */

      /* if found, and a directory, set current lookup directory, and go to
       * next component.
       */

    } else {
    }
  }

  return explained;
}
