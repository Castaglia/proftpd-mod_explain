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

/* Generic API tests. */

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

START_TEST (generic_describe_generic_params_test) {
  const char *desc, *syscall;

  desc = explain_describe_generic(NULL, 0, NULL);
  fail_unless(desc == NULL, "Failed to handle null pool");
  fail_unless(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  desc = explain_describe_generic(p, 0, NULL);
  fail_unless(desc == NULL, "Failed to handle null syscall");
  fail_unless(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  syscall = "write(2)";
  desc = explain_describe_generic(p, 0, syscall);
  fail_unless(desc == NULL, "Failed to handle errno zero");
  fail_unless(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
}
END_TEST

START_TEST (generic_describe_generic_eagain_test) {
#ifdef EAGAIN
  const char *desc, *syscall, *expected;
  int xerrno;

  syscall = "write(2)";
  xerrno = EAGAIN;
  desc = explain_describe_generic(p, xerrno, syscall);
  fail_unless(desc != NULL, "Failed to describe %s errno %d: %s", syscall,
    xerrno, strerror(errno));

  expected = "the write(2) system call was waiting to finish but was told not to wait";
  fail_unless(strcmp(desc, expected) == 0,
    "Expected '%s', got '%s'", expected, desc);
#endif /* EAGAIN */
}
END_TEST

START_TEST (generic_describe_generic_default_test) {
  const char *desc, *syscall, *expected;
  int xerrno;

  syscall = "write(2)";
  xerrno = ENOTSOCK;
  desc = explain_describe_generic(p, xerrno, syscall);
  fail_unless(desc != NULL, "Failed to describe %s errno %d: %s", syscall,
    xerrno, strerror(errno));

  expected = "the entropic gremlins have frobnicated the write(2) system call";
  fail_unless(strcmp(desc, expected) == 0,
    "Expected '%s', got '%s'", expected, desc);
}
END_TEST

Suite *tests_get_generic_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("generic");
  testcase = tcase_create("base");

  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, generic_describe_generic_params_test);
  tcase_add_test(testcase, generic_describe_generic_eagain_test);
#if 0
  tcase_add_test(testcase, generic_describe_generic_emfile_test);
  tcase_add_test(testcase, generic_describe_generic_enfile_test);
  tcase_add_test(testcase, generic_describe_generic_eintr_test);
  tcase_add_test(testcase, generic_describe_generic_efault_test);
  tcase_add_test(testcase, generic_describe_generic_eio_test);
  tcase_add_test(testcase, generic_describe_generic_enomem_test);
  tcase_add_test(testcase, generic_describe_generic_eperm_test);
#endif
  tcase_add_test(testcase, generic_describe_generic_default_test);

  suite_add_tcase(suite, testcase);
  return suite;
}
