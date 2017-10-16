package ProFTPD::Tests::Modules::mod_explain::unlink;

use lib qw(t/lib);
use base qw(ProFTPD::TestSuite::Child);
use strict;

use File::Path qw(mkpath);
use File::Spec;
use IO::Handle;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  explain_unlink_enoent_dir => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  explain_unlink_enoent_file => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  explain_unlink_eacces_dir => {
    order => ++$order,
    test_class => [qw(forking rootprivs)],
  },

  explain_unlink_eacces_file => {
    order => ++$order,
    test_class => [qw(forking rootprivs)],
  },

  # parent directory has sticky bit, file not owned by deleter.  Not working
  # as expected.
  explain_unlink_eperm_file => {
    order => ++$order,
    test_class => [qw(forking rootprivs)],
  },

  # erofs [read-only filesystem]
};

sub new {
  return shift()->SUPER::new(@_);
}

sub list_tests {
  return testsuite_get_runnable_tests($TESTS);
}

sub explain_unlink_enoent_dir {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'explain');

  # Deliberately choose a path which does not exist.
  my $test_file = '/tmp/foo/bar/baz/quxx';

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    DebugLevel => 3,
    TraceLog => $setup->{log_file},
    Trace => 'explain:20 explain.path:20 error:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

  # Open pipes, for use between the parent and child processes.  Specifically,
  # the child will indicate when it's done with its test by writing a message
  # to the parent.
  my ($rfh, $wfh);
  unless (pipe($rfh, $wfh)) {
    die("Can't open pipe: $!");
  }

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      eval { $client->dele($test_file) };
      unless ($@) {
        die("DELE $test_file succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected = 550;
      $self->assert($expected == $resp_code,
        "Expected response code $expected, got $resp_code");

      $expected = "$test_file: No such file or directory";
      $self->assert($expected eq $resp_msg,
        "Expected response message '$expected', got '$resp_msg'");
    };
    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($setup->{config_file}, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($setup->{pid_file});
  $self->assert_child_ok($pid);

  eval {
    if (open(my $fh, "< $setup->{log_file}")) {
      my $ok = 0;

      while (my $line = <$fh>) {
        chomp($line);

        if ($ENV{TEST_VERBOSE}) {
          print STDERR "# $line\n";
        }

        if ($line =~ /because directory .*? does not exist/) {
          $ok = 1;
          last;
        }
      }

      close($fh);
      $self->assert($ok, "Did not see expected explanation in log messages");

    } else {
      die("Can't read $setup->{log_file}: $!");
    }
  };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

sub explain_unlink_enoent_file {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'explain');

  my $test_file = File::Spec->rel2abs("$tmpdir/test.dat");

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    DebugLevel => 3,
    TraceLog => $setup->{log_file},
    Trace => 'explain:20 explain.path:20 error:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

  # Open pipes, for use between the parent and child processes.  Specifically,
  # the child will indicate when it's done with its test by writing a message
  # to the parent.
  my ($rfh, $wfh);
  unless (pipe($rfh, $wfh)) {
    die("Can't open pipe: $!");
  }

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      eval { $client->dele($test_file) };
      unless ($@) {
        die("DELE $test_file succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected = 550;
      $self->assert($expected == $resp_code,
        "Expected response code $expected, got $resp_code");

      $expected = "$test_file: No such file or directory";
      $self->assert($expected eq $resp_msg,
        "Expected response message '$expected', got '$resp_msg'");
    };
    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($setup->{config_file}, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($setup->{pid_file});
  $self->assert_child_ok($pid);

  eval {
    if (open(my $fh, "< $setup->{log_file}")) {
      my $ok = 0;

      while (my $line = <$fh>) {
        chomp($line);

        if ($ENV{TEST_VERBOSE}) {
          print STDERR "# $line\n";
        }

        if ($line =~ /because file .*? does not exist/) {
          $ok = 1;
          last;
        }
      }

      close($fh);
      $self->assert($ok, "Did not see expected explanation in log messages");

    } else {
      die("Can't read $setup->{log_file}: $!");
    }
  };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

sub explain_unlink_eacces_dir {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'explain');

  my $test_dir = File::Spec->rel2abs("$tmpdir/test.d/sub.d");
  mkpath($test_dir);

  my $test_file = File::Spec->rel2abs("$test_dir/test.dat");
  if (open(my $fh, "> $test_file")) {
    print $fh "Hello, World!\n";
    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  unless (chmod(0700, $test_dir)) {
    die("Can't set $test_dir perms to 0700: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    DebugLevel => 3,
    TraceLog => $setup->{log_file},
    Trace => 'explain:20 explain.path:20 error:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

  # Open pipes, for use between the parent and child processes.  Specifically,
  # the child will indicate when it's done with its test by writing a message
  # to the parent.
  my ($rfh, $wfh);
  unless (pipe($rfh, $wfh)) {
    die("Can't open pipe: $!");
  }

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      eval { $client->dele($test_file) };
      unless ($@) {
        die("DELE $test_file succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected = 550;
      $self->assert($expected == $resp_code,
        "Expected response code $expected, got $resp_code");

      $expected = "$test_file: Permission denied";
      $self->assert($expected eq $resp_msg,
        "Expected response message '$expected', got '$resp_msg'");
    };
    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($setup->{config_file}, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($setup->{pid_file});
  $self->assert_child_ok($pid);

  eval {
    if (open(my $fh, "< $setup->{log_file}")) {
      my $ok = 0;

      while (my $line = <$fh>) {
        chomp($line);

        if ($ENV{TEST_VERBOSE}) {
          print STDERR "# $line\n";
        }

        if ($line =~ /because (directory|file) .*? is not searchable by the user/) {
          $ok = 1;
          last;
        }
      }

      close($fh);
      $self->assert($ok, "Did not see expected explanation in log messages");

    } else {
      die("Can't read $setup->{log_file}: $!");
    }
  };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

sub explain_unlink_eacces_file {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'explain');

  my $test_dir = File::Spec->rel2abs("$tmpdir/test.d/sub.d");
  mkpath($test_dir);

  my $test_file = File::Spec->rel2abs("$test_dir/test.dat");
  if (open(my $fh, "> $test_file")) {
    print $fh "Hello, World!\n";
    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    DebugLevel => 3,
    TraceLog => $setup->{log_file},
    Trace => 'explain:20 explain.path:20 error:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

  # Open pipes, for use between the parent and child processes.  Specifically,
  # the child will indicate when it's done with its test by writing a message
  # to the parent.
  my ($rfh, $wfh);
  unless (pipe($rfh, $wfh)) {
    die("Can't open pipe: $!");
  }

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      eval { $client->dele($test_file) };
      unless ($@) {
        die("DELE $test_file succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected = 550;
      $self->assert($expected == $resp_code,
        "Expected response code $expected, got $resp_code");

      $expected = "$test_file: Permission denied";
      $self->assert($expected eq $resp_msg,
        "Expected response message '$expected', got '$resp_msg'");
    };
    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($setup->{config_file}, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($setup->{pid_file});
  $self->assert_child_ok($pid);

  eval {
    if (open(my $fh, "< $setup->{log_file}")) {
      my $ok = 0;

      while (my $line = <$fh>) {
        chomp($line);

        if ($ENV{TEST_VERBOSE}) {
          print STDERR "# $line\n";
        }

        if ($line =~ /because directory .*? is not writable by the user/) {
          $ok = 1;
          last;
        }
      }

      close($fh);
      $self->assert($ok, "Did not see expected explanation in log messages");

    } else {
      die("Can't read $setup->{log_file}: $!");
    }
  };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

sub explain_unlink_eperm_file {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'explain');

  my $test_dir = File::Spec->rel2abs("$tmpdir/test.d");
  mkpath($test_dir);

  my $sub_dir = File::Spec->rel2abs("$tmpdir/test.d/sub.d");
  mkpath($sub_dir);

  my $test_file = File::Spec->rel2abs("$sub_dir/test.dat");
  if (open(my $fh, "> $test_file")) {
    print $fh "Hello, World!\n";
    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $perms = '1777';
  unless (chmod(oct($perms), $test_dir, $sub_dir)) {
    die("Can't set perms $perms on $test_dir, $sub_dir: $!");
  }

  unless (chmod(0666, $test_file)) {
    die("Can't set perms 0666 on $test_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    DebugLevel => 3,
    TraceLog => $setup->{log_file},
    Trace => 'explain:20 explain.path:20 error:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

  # Open pipes, for use between the parent and child processes.  Specifically,
  # the child will indicate when it's done with its test by writing a message
  # to the parent.
  my ($rfh, $wfh);
  unless (pipe($rfh, $wfh)) {
    die("Can't open pipe: $!");
  }

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      eval { $client->dele($test_file) };
      unless ($@) {
        die("DELE $test_file succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected = 550;
      $self->assert($expected == $resp_code,
        "Expected response code $expected, got $resp_code");

      $expected = "$test_file: Permission denied";
      $self->assert($expected eq $resp_msg,
        "Expected response message '$expected', got '$resp_msg'");
    };
    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($setup->{config_file}, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($setup->{pid_file});
  $self->assert_child_ok($pid);

  eval {
    if (open(my $fh, "< $setup->{log_file}")) {
      my $ok = 0;

      while (my $line = <$fh>) {
        chomp($line);

        if ($ENV{TEST_VERBOSE}) {
          print STDERR "# $line\n";
        }

        if ($line =~ /because directory .*? is not writable by the user/) {
          $ok = 1;
          last;
        }
      }

      close($fh);
      $self->assert($ok, "Did not see expected explanation in log messages");

    } else {
      die("Can't read $setup->{log_file}: $!");
    }
  };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

1;
