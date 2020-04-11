package ProFTPD::Tests::Modules::mod_explain::open;

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
  explain_open_enoent_dir => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  explain_open_enoent_file => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  explain_open_search_eacces_dir => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  explain_open_write_eacces_dir => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  explain_open_read_eacces_file => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  explain_open_write_eacces_file => {
    order => ++$order,
    test_class => [qw(forking)],
  },

};

sub new {
  return shift()->SUPER::new(@_);
}

sub list_tests {
#  return testsuite_get_runnable_tests($TESTS);

#  Require lstat/stat_with_error support in dir_realpath et al
#    explain_open_enoent_dir
  return qw(
    explain_open_enoent_file
  );

#  explain_open_search_eacces_dir
#  explain_open_write_eacces_dir
#  explain_open_read_eacces_file
#  explain_open_write_eacces_file
}

sub explain_open_enoent_dir {
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
    Trace => 'explain:20 explain.path:20 error:20 fsio:20',

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

      eval { $client->retr($test_file) };
      unless ($@) {
        die("RETR $test_file succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected = 550;
      $self->assert($expected == $resp_code,
        "Expected response code $expected, got $resp_code");

      $expected = "$test_file: No such file or directory";
      $self->assert($expected eq $resp_msg,
        "Expected response message '$expected', got '$resp_msg'");

      $client->quit();
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

sub explain_open_enoent_file {
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
    Trace => 'explain:20 explain.path:20 error:20 fsio:20',

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

      eval { $client->retr($test_file) };
      unless ($@) {
        die("RETR $test_file succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected = 550;
      $self->assert($expected == $resp_code,
        "Expected response code $expected, got $resp_code");

      $expected = "$test_file: No such file or directory";
      $self->assert($expected eq $resp_msg,
        "Expected response message '$expected', got '$resp_msg'");

      $client->quit();
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

1;
