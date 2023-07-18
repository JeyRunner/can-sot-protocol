#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

int main(int argc, char **argv) {
  doctest::Context context;
  context.applyCommandLine(argc, argv);

  int res = context.run(); // run doctest

  // important - query flags (and --exit) rely on the user doing this
  if (context.shouldExit()) {
    // propagate the result of the tests
    return res;
  }
}

// running notes
// ./example_exe --no-run (run normal program)
// ./example_exe --exit (run tests then exit)
// ./example_exe (run tests then run program)
// ./example_exe --success (print successful test casts)