/*
 * ProFTPD - mod_explain testsuite
 * Copyright (c) 2016-2022 TJ Saunders <tj@castaglia.org>
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
  ck_assert_msg(desc == NULL, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  desc = explain_describe_generic(p, 0, NULL);
  ck_assert_msg(desc == NULL, "Failed to handle null syscall");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  syscall = "write(2)";
  desc = explain_describe_generic(p, 0, syscall);
  ck_assert_msg(desc == NULL, "Failed to handle errno zero");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
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
  ck_assert_msg(desc != NULL, "Failed to describe %s errno %d: %s", syscall,
    xerrno, strerror(errno));

  expected = "the write(2) system call was waiting to finish but was told not to wait";
  ck_assert_msg(strcmp(desc, expected) == 0,
    "Expected '%s', got '%s'", expected, desc);
#endif /* EAGAIN */
}
END_TEST

START_TEST (generic_describe_generic_emfile_test) {
#ifdef EMFILE
  const char *desc, *syscall, *expected;
  int xerrno;

  syscall = "write(2)";
  xerrno = EMFILE;
  desc = explain_describe_generic(p, xerrno, syscall);
  ck_assert_msg(desc != NULL, "Failed to describe %s errno %d: %s", syscall,
    xerrno, strerror(errno));

  expected = "the process already has the maximum number of file descriptors open";
  ck_assert_msg(strcmp(desc, expected) == 0,
    "Expected '%s', got '%s'", expected, desc);
#endif /* EMFILE */
}
END_TEST

START_TEST (generic_describe_generic_enfile_test) {
#ifdef ENFILE
  const char *desc, *syscall, *expected;
  int xerrno;

  syscall = "write(2)";
  xerrno = ENFILE;
  desc = explain_describe_generic(p, xerrno, syscall);
  ck_assert_msg(desc != NULL, "Failed to describe %s errno %d: %s", syscall,
    xerrno, strerror(errno));

  expected = "the system limit on the total number of open files has been reached";
  ck_assert_msg(strcmp(desc, expected) == 0,
    "Expected '%s', got '%s'", expected, desc);
#endif /* ENFILE */
}
END_TEST

START_TEST (generic_describe_generic_eintr_test) {
#ifdef EINTR
  const char *desc, *syscall, *expected;
  int xerrno;

  syscall = "write(2)";
  xerrno = EINTR;
  desc = explain_describe_generic(p, xerrno, syscall);
  ck_assert_msg(desc != NULL, "Failed to describe %s errno %d: %s", syscall,
    xerrno, strerror(errno));

  expected = "the write(2) system call was interrupted by a signal before it could finish";
  ck_assert_msg(strcmp(desc, expected) == 0,
    "Expected '%s', got '%s'", expected, desc);
#endif /* EINTR */
}
END_TEST

START_TEST (generic_describe_generic_efault_test) {
#ifdef EFAULT
  const char *desc, *syscall, *expected;
  int xerrno;

  syscall = "write(2)";
  xerrno = EFAULT;
  desc = explain_describe_generic(p, xerrno, syscall);
  ck_assert_msg(desc != NULL, "Failed to describe %s errno %d: %s", syscall,
    xerrno, strerror(errno));

  expected = "one of the write(2) parameters is null, uninitialized, or points to invalid/unreachable memory";
  ck_assert_msg(strcmp(desc, expected) == 0,
    "Expected '%s', got '%s'", expected, desc);
#endif /* EFAULT */
}
END_TEST

START_TEST (generic_describe_generic_eio_test) {
#ifdef EIO
  const char *desc, *syscall, *expected;
  int xerrno;

  syscall = "write(2)";
  xerrno = EIO;
  desc = explain_describe_generic(p, xerrno, syscall);
  ck_assert_msg(desc != NULL, "Failed to describe %s errno %d: %s", syscall,
    xerrno, strerror(errno));

  expected = "a low-level I/O error occurred, probably in hardware";
  ck_assert_msg(strcmp(desc, expected) == 0,
    "Expected '%s', got '%s'", expected, desc);
#endif /* EIO */
}
END_TEST

START_TEST (generic_describe_generic_enomem_test) {
#ifdef ENOMEM
  const char *desc, *syscall, *expected;
  int xerrno;

  syscall = "write(2)";
  xerrno = ENOMEM;
  desc = explain_describe_generic(p, xerrno, syscall);
  ck_assert_msg(desc != NULL, "Failed to describe %s errno %d: %s", syscall,
    xerrno, strerror(errno));

  expected = "there was not enough user-space memory available for write(2) to succeed";
  ck_assert_msg(strcmp(desc, expected) == 0,
    "Expected '%s', got '%s'", expected, desc);
#endif /* ENOMEM */
}
END_TEST

START_TEST (generic_describe_generic_eperm_test) {
#ifdef EPERM
  const char *desc, *syscall, *expected;
  int xerrno;

  syscall = "write(2)";
  xerrno = EPERM;
  desc = explain_describe_generic(p, xerrno, syscall);
  ck_assert_msg(desc != NULL, "Failed to describe %s errno %d: %s", syscall,
    xerrno, strerror(errno));

  expected = "the process does not have the appropriate privileges to use the write(2) system call";
  ck_assert_msg(strcmp(desc, expected) == 0,
    "Expected '%s', got '%s'", expected, desc);
#endif /* EPERM */
}
END_TEST

START_TEST (generic_describe_generic_default_test) {
  const char *desc, *syscall, *expected;
  int xerrno;

  syscall = "write(2)";
  xerrno = ENOTSOCK;
  desc = explain_describe_generic(p, xerrno, syscall);
  ck_assert_msg(desc != NULL, "Failed to describe %s errno %d: %s", syscall,
    xerrno, strerror(errno));

  expected = "the entropic gremlins have frobnicated the write(2) system call";
  ck_assert_msg(strcmp(desc, expected) == 0,
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
  tcase_add_test(testcase, generic_describe_generic_emfile_test);
  tcase_add_test(testcase, generic_describe_generic_enfile_test);
  tcase_add_test(testcase, generic_describe_generic_eintr_test);
  tcase_add_test(testcase, generic_describe_generic_efault_test);
  tcase_add_test(testcase, generic_describe_generic_eio_test);
  tcase_add_test(testcase, generic_describe_generic_enomem_test);
  tcase_add_test(testcase, generic_describe_generic_eperm_test);
  tcase_add_test(testcase, generic_describe_generic_default_test);

  suite_add_tcase(suite, testcase);
  return suite;
}
