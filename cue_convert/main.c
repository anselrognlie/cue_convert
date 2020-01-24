//#define MEMCHECK

#ifdef MEMCHECK
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <stdio.h>

#include "cue_parser.h"
#include "cue_file.h"

int main(int argc, char const *argv[]) {

#ifdef MEMCHECK
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  //printf("hello world\n");
  //printf("argc: %d\n", argc);
  //for (int i = 0; i < argc; ++i) {
  //  printf("argv[%d]: %s\n", i, argv[i]);
  //}

  // argv[0] = exe name
  char const *exe = argv[0];

  // argv[1] = source cue name
  char const *cue = argv[1];

  if (argc < 2) {
    printf("usage: %s <cue_file>\n", exe);
    return 0;
  }

  cue_sheet_t *sheet = cue_sheet_parse_filename(cue);
  if (sheet) {
    cue_sheet_write_file(sheet, stdout);
    cue_sheet_free(sheet);
  }

  return 0;
}
