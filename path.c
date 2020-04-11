/*
 * ProFTPD - mod_explain: path_resolution(2)
 * Copyright (c) 2016-2020 TJ Saunders
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

static const char *trace_channel = "explain.path";

static const char *mode2text(pool *p, mode_t o) {
  char buf[1024];
  memset(buf, '\0', sizeof(buf));
  snprintf(buf, sizeof(buf)-1, "%04o", (int) (o & ~S_IFMT));
  return pstrdup(p, buf);
}

static const char *ul2text(pool *p, unsigned long l) {
  char buf[1024];
  memset(buf, '\0', sizeof(buf));
  snprintf(buf, sizeof(buf)-1, "%lu", l);
  return pstrdup(p, buf);
}

static array_header *path_split(pool *p, const char *path) {
  char buf[PR_TUNABLE_PATH_MAX+1], *full_path;
  size_t full_pathlen;
  array_header *first, *components;

  pr_fs_virtual_path(path, buf, sizeof(buf)-1);
  full_path = buf;
  full_pathlen = strlen(full_path);

  /* If the path ENDS in '/', append '.' to it. */
  if (full_path[full_pathlen-1] == '/' &&
      full_pathlen < (sizeof(buf)-1)) {
    full_path[full_pathlen] = '.';
  }

  first = make_array(p, 1, sizeof(char *));

  /* The first component is ALWAYS '/'. */
  *((char **) push_array(first)) = pstrdup(p, "/");

  if (full_pathlen == 1) {
    return first;
  }

  components = pr_str_text_to_array(p, full_path, '/');
  if (components != NULL) {
    components = append_arrays(p, first, components);
  }

  pr_trace_msg(trace_channel, 19, "split path '%s' into %u components", path,
    components->nelts);
  return components;
}

static const char *describe_enametoolong_name(pool *p, const char *name,
    size_t name_len, unsigned long name_max, int flags) {
  return pstrcat(p, "path component '", name,
    "' exceeds the system maximum name length (",
    ul2text(p, (unsigned long) name_len), " > max ", ul2text(p, name_max), ")",
    NULL);
}

static const char *describe_enametoolong_path(pool *p, const char *path,
    size_t path_len, unsigned long path_max, int flags) {
  return pstrcat(p, "'", path, "' exceeds the system maximum path length (",
    ul2text(p, (unsigned long) path_len), " > max ", ul2text(p, path_max), ")",
    NULL);
}

static const char *describe_eacces_dir(pool *p, const char *path, int flags) {
  return pstrcat(p, "directory '", path, "' is not searchable by the user",
    NULL);
}

static const char *describe_eacces_file(pool *p, const char *path, int flags) {
  if (flags & EXPLAIN_PATH_FL_WANT_SEARCH) {
    return pstrcat(p, "file '", path, "' is not searchable by the user", NULL);
  }

  return pstrcat(p, "directory containing '", path,
    "' is not writable by the user", NULL);
}

static const char *describe_enoent_dir(pool *p, const char *path, int flags) {
  return pstrcat(p, "directory '", path, "' does not exist", NULL);
}

static const char *describe_enoent_file(pool *p, const char *path, int flags) {
  return pstrcat(p, "file '", path, "' does not exist", NULL);
}

const char *explain_path_error(pool *p, int err_errno, const char *full_path,
    int flags, mode_t mode) {
  register unsigned int i;
  array_header *components = NULL;
  const char *explained = NULL, *path = NULL, *prev_path = NULL;
  unsigned long name_max, no_trunc;

  if (p == NULL ||
      full_path == NULL) {
    errno = EINVAL;
    return NULL;
  }

  /* Try to get some of the easy cases out of the way first. */

  if (err_errno == ENAMETOOLONG) {
    unsigned long path_max;

    path_max = explain_platform_path_max(p, full_path);
    if (path_max > 0) {
      size_t path_len;

      path_len = strlen(full_path);
      if (path_len > path_max) {
        explained = describe_enametoolong_path(p, full_path, path_len,
          path_max, flags);
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
    int final_component = FALSE, res, xerrno = 0;
    struct stat st;

    pr_signals_handle();

    elts = components->elts;
    component = elts[i];

    if (err_errno == ENAMETOOLONG) {
      size_t component_len;

      component_len = strlen(component);

      /* Component names can be too long only if the filesystem on which
       * the path resides does not silently truncate long names.
       */
      if (component_len > name_max &&
          no_trunc == 1) {
        explained = describe_enametoolong_name(p, component, component_len,
          name_max, flags);
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

    res = pr_fsio_lstat(path, &st);
    xerrno = errno;

    if (res < 0) {
      pr_trace_msg(trace_channel, 3,
        "error checking component #%u (of %u), path '%s': %s", i+1,
        components->nelts, path, strerror(xerrno));
    }

    if (final_component == FALSE) {
      if (res < 0) {
        switch (xerrno) {
          case ENOENT:
            explained = describe_enoent_dir(p, path, flags);
            break;

          case EACCES:
            explained = describe_eacces_dir(p, path, flags);
            break;

          default:
            pr_trace_msg(trace_channel, 3,
              "unexplained error [%s (%d)] for directory '%s'",
              strerror(xerrno), errno, path);
        }

        break;
      }

      /* XXX What if this is a symlink? */

      if (!S_ISLNK(st.st_mode) &&
          !S_ISDIR(st.st_mode)) {
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
      /* Last component, full path, leaf file. */
      if (res < 0) {
        switch (xerrno) {
          case ENOENT:
            explained = describe_enoent_file(p, path, flags);
            break;

          case EACCES:
            explained = describe_eacces_file(p, path, flags);
            break;

          default:
            pr_trace_msg(trace_channel, 3,
              "unexplained error [%s (%d)] for file '%s'",
              strerror(xerrno), errno, path);
        }

        break;
      }

      switch (err_errno) {
        case EACCES:
          explained = describe_eacces_file(p, path, flags);
          if (explained != NULL) {
            if (pr_fsio_lstat(prev_path, &st) == 0) {
              explained = pstrcat(p, explained, "; parent directory '",
                prev_path, "' has perms ", mode2text(p, st.st_mode),
                ", and is owned by UID ", pr_uid2str(p, st.st_uid),
                ", GID ", pr_gid2str(p, st.st_gid), NULL);
            }
          }

          break;

        default:
          pr_trace_msg(trace_channel, 3,
            "unexplained error [%s (%d)] for file '%s'",
            strerror(xerrno), errno, path);
      }
    }

    prev_path = path;
  }

  return explained;
}
