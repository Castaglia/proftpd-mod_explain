/*
 * ProFTPD - mod_explain testsuite
 * Copyright (c) 2016-2017 TJ Saunders <tj@castaglia.org>
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

/* Path API tests. */

#include "tests.h"
#include "path.h"

static pool *p = NULL;

static void set_up(void) {
  if (p == NULL) {
    p = make_sub_pool(NULL);
  }
}

static void tear_down(void) {
  if (p) {
    destroy_pool(p);
    p = NULL;
  } 
}

START_TEST (path_error_params_test) {
  const char *desc;

  desc = explain_path_error(NULL, 0, NULL, 0, 0);
  fail_unless(desc == NULL, "Failed to handle null pool");
  fail_unless(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  desc = explain_path_error(p, 0, NULL, 0, 0);
  fail_unless(desc == NULL, "Failed to handle null path");
  fail_unless(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
}
END_TEST

Suite *tests_get_path_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("path");
  testcase = tcase_create("base");

  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, path_error_params_test);

/* XXX Tests to add:
 *  ENOENT (component, name)
 *  ENAMETOOLONG (path, component)
 *  EACCES (component, name)
 *  ENOTDIR (component)
 *
 * And the errors in combination with the access flags
 */

  suite_add_tcase(suite, testcase);
  return suite;
}
