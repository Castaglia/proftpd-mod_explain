/*
 * ProFTPD - mod_explain
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
 *
 * -----DO NOT EDIT BELOW THIS LINE-----
 * $Archive: mod_explain.a$
 */

#include "mod_explain.h"
#include "platform.h"
#include "chroot.h"

extern xaset_t *server_list;

module explain_module;

pool *explain_pool = NULL;
unsigned long explain_opts = 0UL;

static int explain_engine = TRUE;

static const char *trace_channel = "explain";

/* Error explanation providers. */

static const char *explain_chmod(pool *p, int xerrno, const char *path,
    mode_t mode, const char **args) {
  errno = ENOSYS;
  return NULL;
}

static const char *explain_chown(pool *p, int xerrno, const char *path,
    uid_t uid, gid_t gid, const char **args) {
  errno = ENOSYS;
  return NULL;
}

static const char *explain_chroot(pool *p, int xerrno, const char *path,
    const char **args) {
  return explain_chroot_error(p, xerrno, path, args);
}

static const char *explain_close(pool *p, int xerrno, int fd,
    const char **args) {
  errno = ENOSYS;
  return NULL;
}

static const char *explain_fchmod(pool *p, int xerrno, int fd, mode_t mode,
    const char **args) {
  errno = ENOSYS;
  return NULL;
}

static const char *explain_fchown(pool *p, int xerrno, int fd, uid_t uid,
    gid_t gid, const char **args) {
  errno = ENOSYS;
  return NULL;
}

static const char *explain_lchown(pool *p, int xerrno, const char *path,
    uid_t uid, gid_t gid, const char **args) {
  errno = ENOSYS;
  return NULL;
}

static const char *explain_mkdir(pool *p, int xerrno, const char *path,
    mode_t mode, const char **args) {
  errno = ENOSYS;
  return NULL;
}

static const char *explain_open(pool *p, int xerrno, const char *path,
    int flags, mode_t mode, const char **args) {
  errno = ENOSYS;
  return NULL;
}

static const char *explain_read(pool *p, int xerrno, int fd, void *buf,
    size_t sz, const char **args) {
  errno = ENOSYS;
  return NULL;
}

static const char *explain_rename(pool *p, int xerrno, const char *old_path,
    const char *new_path, const char **args) {
  errno = ENOSYS;
  return NULL;
}

static const char *explain_rmdir(pool *p, int xerrno, const char *path,
    const char **args) {
  errno = ENOSYS;
  return NULL;
}

static const char *explain_stat(pool *p, int xerrno, const char *path,
    struct stat *st, const char **args) {
  errno = ENOSYS;
  return NULL;
}

static const char *explain_unlink(pool *p, int xerrno, const char *path,
    const char **args) {
  errno = ENOSYS;
  return NULL;
}

static const char *explain_write(pool *p, int xerrno, int fd,
    const void *buf, size_t sz, const char **args) {
  errno = ENOSYS;
  return NULL;
}

/* Configuration directives
 */

/* usage: ExplainEngine on|off */
MODRET set_explainengine(cmd_rec *cmd) {
  int engine = -1;

  CHECK_CONF(cmd, CONF_ROOT);
  CHECK_ARGS(cmd, 1);

  engine = get_boolean(cmd, 1);
  if (engine < 0) {
    CONF_ERROR(cmd, "expected Boolean parameter");
  }

  explain_engine = engine;
  return PR_HANDLED(cmd);
}

/* usage: ExplainOptions opt1 ... */
MODRET set_explainoptions(cmd_rec *cmd) {
  register unsigned int i;
  unsigned long opts = 0UL;
  config_rec *c;

  if (cmd->argc < 2) {
    CONF_ERROR(cmd, "wrong number of parameters");
  }

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  c = add_config_param(cmd->argv[0], 1, NULL);

  for (i = 1; i < cmd->argc; i++) {
    if (strcmp(cmd->argv[i], "Foo") == 0) {

    } else {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unknown ExplainOption '",
        cmd->argv[i], "'", NULL));
    }
  }

  c->argv[0] = palloc(c->pool, sizeof(unsigned long));
  *((unsigned long *) c->argv[0]) = opts;

  return PR_HANDLED(cmd);
}

/* Event listeners
 */

#if defined(PR_SHARED_MODULE)
static void explain_mod_unload_ev(const void *event_data, void *user_data) {
  if (strcmp((const char *) event_data, "mod_explain.c") == 0) {
    /* Unregister ourselves from all events. */
    pr_event_unregister(&explain_module, NULL, NULL);

    /* Remove our explanations. */
    (void) pr_error_unregister_explanations(explain_pool, &explain_module,
      "explain");

    destroy_pool(explain_pool);
    explain_pool = NULL;
  }
}
#endif /* PR_SHARED_MODULE */

static void explain_postparse_ev(const void *event_data, void *user_data) {
  if (explain_engine == TRUE) {
    pr_error_explanations_t *explanations;

    explanations = pr_error_register_explanations(explain_pool,
      &explain_module, "explain");
    if (explanations != NULL) {
      explanations->explain_chmod = explain_chmod;
      explanations->explain_chown = explain_chown;
      explanations->explain_chroot = explain_chroot;
      explanations->explain_close = explain_close;
      explanations->explain_fchmod = explain_fchmod;
      explanations->explain_fchown = explain_fchown;
      explanations->explain_lchown = explain_lchown;
      explanations->explain_mkdir = explain_mkdir;
      explanations->explain_open = explain_open;
      explanations->explain_read = explain_read;
      explanations->explain_rename = explain_rename;
      explanations->explain_rmdir = explain_rmdir;
      explanations->explain_stat = explain_stat;
      explanations->explain_unlink = explain_unlink;
      explanations->explain_write = explain_write;

    } else {
      pr_trace_msg(trace_channel, 1, "error registering error explanations: %s",
        strerror(errno));
      explain_engine = FALSE;
    }
  }
}

static void explain_restart_ev(const void *event_data, void *user_data) {
  destroy_pool(explain_pool);
  explain_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(explain_pool, MOD_EXPLAIN_VERSION);

  /* Reset default ExplainEngine setting. */
  explain_engine = TRUE;
}

static void explain_shutdown_ev(const void *event_data, void *user_data) {
  destroy_pool(explain_pool);
  explain_pool = NULL;
}

/* XXX Do we want to support any Controls/ftpctl actions? */

/* Initialization routines
 */

static int explain_init(void) {
  explain_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(explain_pool, MOD_EXPLAIN_VERSION);

#if defined(PR_SHARED_MODULE)
  pr_event_register(&explain_module, "core.module-unload",
    explain_mod_unload_ev, NULL);
#endif
  pr_event_register(&explain_module, "core.postparse", explain_postparse_ev,
    NULL);
  pr_event_register(&explain_module, "core.restart", explain_restart_ev, NULL);
  pr_event_register(&explain_module, "core.shutdown", explain_shutdown_ev,
    NULL);

  return 0;
}

static int explain_sess_init(void) {
  config_rec *c;

  if (explain_engine == FALSE) {
    return 0;
  }

  explain_platform_init(session.pool);

  /* XXX If/when we have explained responses, they should enabled/disabled
   * on a per-session basis.  Best in a POST_CMD PASS handler, though.
   */

  c = find_config(main_server->conf, CONF_PARAM, "ExplainOptions", FALSE);
  while (c != NULL) {
    unsigned long opts;

    pr_signals_handle();

    opts = *((unsigned long *) c->argv[0]);
    explain_opts |= opts;

    c = find_config_next(c, c->next, CONF_PARAM, "ExplainOptions", FALSE);
  }

  return 0;
}

/* Module API tables
 */

static conftable explain_conftab[] = {
  { "ExplainEngine",		set_explainengine,		NULL },
  { "ExplainOptions",		set_explainoptions,		NULL },

  { NULL }
};

module explain_module = {
  /* Always NULL */
  NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "explain",

  /* Module configuration handler table */
  explain_conftab,

  /* Module command handler table */
  NULL,

  /* Module authentication handler table */
  NULL,

  /* Module initialization */
  explain_init,

  /* Session initialization */
  explain_sess_init,

  /* Module version */
  MOD_EXPLAIN_VERSION
};
