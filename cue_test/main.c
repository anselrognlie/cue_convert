#ifdef _DEBUG
#define MEMCHECK
#endif

#ifdef MEMCHECK
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "all_tests.h"

int main(int argc, char const* argv[]) {
#ifdef MEMCHECK
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
  
  test_getline();
  test_cue();
  test_cue_copy();
  test_cue_transform();
}
