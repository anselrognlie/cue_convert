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

int main(int argc, char const* argv[]) {
#ifdef MEMCHECK
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  test_string_join();
  test_getline();
  test_cue();
  test_cue_copy();
  test_cue_transform();
  test_list_dir();
  test_traverse_dirs();
  test_ensure_path();
  test_enumerate_path();
  test_parallel_traverse();
}
