//#define MEMCHECK

#ifdef MEMCHECK
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <stdio.h>

#include "cue_convert.h"

int main(int argc, char const *argv[]) {

#ifdef MEMCHECK
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  cue_convert_env_t env;

  env.out = stdout;
  env.err = stderr;

  return cue_convert_with_args(argc, argv, &env);
}
