#ifdef _DEBUG
#define MEMCHECK
#endif

#ifdef MEMCHECK
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "all_tests.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>

int main(int argc, char const* argv[]) {
#ifdef MEMCHECK
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  errno_t result = 0;

  result = test_string_join() || result;
  result = test_getline() || result;
  result = test_cue() || result;
  result = test_cue_copy() || result;
  result = test_cue_transform() || result;
  result = test_list_dir() || result;
  result = test_traverse_dirs() || result;
  result = test_ensure_path() || result;
  result = test_enumerate_path() || result;
  result = test_string_stack() || result;
  result = test_string_unstack() || result;
  result = test_string_queue() || result;
  result = test_parallel_traverse() || result;

  printf("%s\n", result ? "FAILURE!" : "All passed.");
}
