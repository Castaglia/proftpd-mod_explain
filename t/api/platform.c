/*
 * ProFTPD - mod_explain testsuite
 * Copyright (c) 2016 TJ Saunders <tj@castaglia.org>
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

/* Platform API tests. */

#include "tests.h"

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

START_TEST (platform_child_max_test) {
  long res, expected;

  explain_platform_init(p);

  expected = sysconf(_SC_CHILD_MAX);
  res = explain_platform_child_max(p);
  fail_unless(res == expected, "Expected %ld, got %ld", expected, res);

  explain_platform_free(p);
}
END_TEST

START_TEST (platform_iov_max_test) {
  long res, expected;

  explain_platform_init(p);

  expected = sysconf(_SC_IOV_MAX);
  res = explain_platform_iov_max(p);
  fail_unless(res == expected, "Expected %ld, got %ld", expected, res);

  explain_platform_free(p);
}
END_TEST

START_TEST (platform_name_max_test) {
  long res;
  const char *path;

  explain_platform_init(p);

  res = explain_platform_name_max(p, NULL);
  fail_unless(res < 0, "Failed to handle null path");
  fail_unless(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  path = "/tmp";
  res = explain_platform_name_max(p, path);
  if (res < 0) {
    fail_unless(errno == ENOSYS, "Expected ENOSYS (%d), got %s (%d)", ENOSYS,
      strerror(errno), errno);
  }

  pr_event_generate("core.chroot", path);

  res = explain_platform_name_max(p, path);
  if (res < 0) {
    fail_unless(errno == ENOSYS, "Expected ENOSYS (%d), got %s (%d)", ENOSYS,
      strerror(errno), errno);
  }

  explain_platform_free(p);
}
END_TEST

START_TEST (platform_no_trunc_test) {
  long res;
  const char *path;

  explain_platform_init(p);

  res = explain_platform_no_trunc(p, NULL);
  fail_unless(res < 0, "Failed to handle null path");
  fail_unless(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  path = "/tmp";
  res = explain_platform_no_trunc(p, path);
  if (res < 0) {
    fail_unless(errno == ENOSYS, "Expected ENOSYS (%d), got %s (%d)", ENOSYS,
      strerror(errno), errno);
  }

  pr_event_generate("core.chroot", path);

  res = explain_platform_no_trunc(p, path);
  if (res < 0) {
    fail_unless(errno == ENOSYS, "Expected ENOSYS (%d), got %s (%d)", ENOSYS,
      strerror(errno), errno);
  }

  explain_platform_free(p);
}
END_TEST

START_TEST (platform_path_max_test) {
  long res;
  const char *path;

  explain_platform_init(p);

  res = explain_platform_path_max(p, NULL);
  fail_unless(res < 0, "Failed to handle null path");
  fail_unless(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  path = "/tmp";
  res = explain_platform_path_max(p, path);
  if (res < 0) {
    fail_unless(errno == ENOSYS, "Expected ENOSYS (%d), got %s (%d)", ENOSYS,
      strerror(errno), errno);
  }

  pr_event_generate("core.chroot", path);

  res = explain_platform_path_max(p, path);
  if (res < 0) {
    fail_unless(errno == ENOSYS, "Expected ENOSYS (%d), got %s (%d)", ENOSYS,
      strerror(errno), errno);
  }

  explain_platform_free(p);
}
END_TEST

START_TEST (platform_open_max_test) {
  long res, expected;

  explain_platform_init(p);

  expected = sysconf(_SC_OPEN_MAX);
  res = explain_platform_open_max(p);
  fail_unless(res == expected, "Expected %ld, got %ld", expected, res);

  explain_platform_free(p);
}
END_TEST

Suite *tests_get_platform_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("platform");
  testcase = tcase_create("base");

  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, platform_child_max_test);
  tcase_add_test(testcase, platform_iov_max_test);
  tcase_add_test(testcase, platform_name_max_test);
  tcase_add_test(testcase, platform_no_trunc_test);
  tcase_add_test(testcase, platform_path_max_test);
  tcase_add_test(testcase, platform_open_max_test);

  suite_add_tcase(suite, testcase);
  return suite;
}
