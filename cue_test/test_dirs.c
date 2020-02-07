#include "all_tests.h"

#include <stdio.h>
#include <string.h>

#include "filesystem.h"
#include "directory_traversal.h"
#include "directory_traversal_handler.h"
#include "path.h"
#include "string_vector.h"
#include "string_helpers.h"
#include "mem_helpers.h"
#include "err_helpers.h"
#include "char_vector.h"
#include "test_visitors.h"

static const char s_test_dir[] = "..\\test_data\\test_dir";
static const char s_test_delete_dir[] = "..\\test_data\\ensure_dir";
static const char s_test_ensure_dir[] = "..\\test_data\\ensure_dir\\a\\dir\\to\\ensure";
static const char s_parallel_dir[] = "p:\\parallel_path";
static const char s_copy_dir[] = "..\\test_data\\copy_dir";

//#define PRINT_ONLY

static const dir_entry_fields_t s_test_list_dir_result[] = {
  {"dir01", 1},
  {"dir02", 1},
  {"file01", 0},
  {"file02", 0},
};

static const size_t s_test_list_dir_result_len =
  sizeof(s_test_list_dir_result) / sizeof(*s_test_list_dir_result);

static const dir_entry_fields_t s_test_traverse_result[] = {
  {"dir01", 1},
  {"file0101", 0},
  {"file0102", 0},
  {"dir02", 1},
  {"file0201", 0},
  {"file0202", 0},
  {"file01", 0},
  {"file02", 0},
};

static const size_t s_test_traverse_result_len =
  sizeof(s_test_traverse_result) / sizeof(*s_test_traverse_result);

static const dir_entry_fields_t s_test_ensure_result[] = {
  {"a", 1},
  {"dir", 1},
  {"to", 1},
  {"ensure", 1},
};

static const size_t s_test_ensure_result_len =
sizeof(s_test_ensure_result) / sizeof(*s_test_ensure_result);

static char const *s_path_enum_paths[] = {
  "r:\\emu\\roms\\pcecd\\a\\akumajo",
  "\\emu\\roms\\pcecd\\a\\akumajo",
  "r:\\emu\\.\\.\\.\\akumajo",
  "r:\\emu\\.\\..\\.\\akumajo",
};

static char const* s_path_enum_paths_0[] = { "r:\\emu", "r:\\emu\\roms", "r:\\emu\\roms\\pcecd", "r:\\emu\\roms\\pcecd\\a", "r:\\emu\\roms\\pcecd\\a\\akumajo", NULL };
static char const* s_path_enum_paths_1[] = { "\\emu", "\\emu\\roms", "\\emu\\roms\\pcecd", "\\emu\\roms\\pcecd\\a", "\\emu\\roms\\pcecd\\a\\akumajo", NULL };
static char const* s_path_enum_paths_2[] = { "r:\\emu", "r:\\emu\\.\\.\\.\\akumajo", NULL };
static char const* s_path_enum_paths_3[] = { "r:\\emu", "r:\\emu\\.\\..\\.\\akumajo", NULL };

static char const** s_path_enum_path_parts[] = {
  s_path_enum_paths_0,
  s_path_enum_paths_1,
  s_path_enum_paths_2,
  s_path_enum_paths_3,
};

static char const* s_parallel_dirs[] = {
  "p:\\parallel_path\\dir01",
  "p:\\parallel_path\\dir01\\file0101",
  "p:\\parallel_path\\dir01\\file0102",
  "p:\\parallel_path\\dir02",
  "p:\\parallel_path\\dir02\\file0201",
  "p:\\parallel_path\\dir02\\file0202",
  "p:\\parallel_path\\file01",
  "p:\\parallel_path\\file02",
};

//
// tests
//

void test_traverse_dirs_print(void) {
  print_visitor_t visitor;
  print_visitor_init(&visitor);

  traverse_dir_path(s_test_dir, &visitor.handler_i);
}

errno_t test_traverse_dirs(void) {
#ifndef PRINT_ONLY
  compare_visitor_t visitor;
  compare_visitor_init(&visitor, s_test_traverse_result, s_test_traverse_result_len);

  printf("Checking traversal of directory %s... ", s_test_dir);

  traverse_dir_path(s_test_dir, &visitor.handler_i);

  if (visitor.line != s_test_traverse_result_len) visitor.passed = 0;

  printf("%s\n", visitor.passed ? "passed." : "FAILED!");

  return !visitor.passed;
#else
  test_traverse_dirs_print();
#endif
}

void test_list_dir_print(void) {
  print_visitor_t visitor;
  print_visitor_init(&visitor);

  directory_traversal_options_t opts;
  memset(&opts, 0, sizeof(opts));

  traverse_dir_path_opts(s_test_dir, &opts, &visitor.handler_i);
}

errno_t test_list_dir(void) {
#ifndef PRINT_ONLY
  compare_visitor_t visitor;
  compare_visitor_init(&visitor, s_test_list_dir_result, s_test_list_dir_result_len);

  directory_traversal_options_t opts;
  memset(&opts, 0, sizeof(opts));

  printf("Checking listing of directory %s... ", s_test_dir);

  traverse_dir_path_opts(s_test_dir, &opts, &visitor.handler_i);

  if (visitor.line != s_test_list_dir_result_len) visitor.passed = 0;

  printf("%s\n", visitor.passed ? "passed." : "FAILED!");

  return ! visitor.passed;
#else
  test_list_dir_print();
#endif
}

static void print_path_enumeration(char const* path) {
  path_enumerator_i* i = enumerate_path(path);
  while (i->has_next(i)) {
    path_enumerate_status_t status = i->next(i);
    printf("%.*s\n", status.path_end - status.full_path_start, status.full_path_start);
  }

  i->dispose(i);
}

errno_t test_ensure_path(void) {
  compare_visitor_t visitor;
  compare_visitor_init(&visitor, s_test_ensure_result, s_test_ensure_result_len);

  printf("Checking ensure path %s... ", s_test_dir);

  if (file_exists(s_test_delete_dir)) {
    delete_dir(s_test_delete_dir);
  }

  if (file_exists(s_test_delete_dir)) {
    printf("%s\n", "FAILED!");
    return -1;
  }

  ensure_dir(s_test_ensure_dir);
  traverse_dir_path(s_test_delete_dir, &visitor.handler_i);

  if (visitor.line != s_test_ensure_result_len) visitor.passed = 0;

  delete_dir(s_test_delete_dir);
  if (file_exists(s_test_delete_dir)) {
    printf("%s\n", "FAILED!");
    return -1;
  }

  printf("%s\n", visitor.passed ? "passed." : "FAILED!");

  return visitor.passed ? 0 : -1;
}

static short check_path_enumeration(char const* path, char const* parts[]) {
  short passed = 1;
  size_t p = 0;
  path_enumerator_i* i = enumerate_path(path);
  while (i->has_next(i)) {
    path_enumerate_status_t status = i->next(i);
    size_t path_len = status.path_end - status.full_path_start;
    size_t part_len = strlen(parts[p]);

    if (path_len != part_len) {
      passed = 0;
      break;
    }

    if (strncmp(status.full_path_start, parts[p], path_len) != 0) {
      passed = 0;
      break;
    }

    ++p;
  }

  i->dispose(i);

  if (parts[p] != NULL) passed = 0;

  return passed;
}

errno_t test_enumerate_path(void) {
  errno_t result = 0;
  size_t num_paths = sizeof(s_path_enum_paths) / sizeof(*s_path_enum_paths);

  printf("Checking path enumeration... ");

  for (size_t i = 0; i < num_paths; ++i) {
    if (!check_path_enumeration(s_path_enum_paths[i], s_path_enum_path_parts[i])) {
      result = -1;
      break;
    }
  }

  printf("%s\n", result ? "FAILED!" : "passed.");

  return result;
}

errno_t test_parallel_traverse(void) {
  parallel_traverse_visitor_t visitor;

  printf("Checking parallel path enumeration... ");

  size_t result_len = sizeof(s_parallel_dirs) / sizeof(*s_parallel_dirs);
  parallel_traverse_visitor_init(&visitor, s_parallel_dir, s_parallel_dirs, result_len);

  traverse_dir_path(s_test_dir, &visitor.pv_t.handler_i);

  parallel_traverse_visitor_uninit(&visitor);

  printf("%s\n", visitor.passed ? "passed." : "FAILED!");

  return visitor.passed ? 0 : -1;
}

errno_t test_copy_dir(void) {
  errno_t err = 0;

  printf("Checking directory copying... ");

  compare_visitor_t visitor;

  ERR_REGION_BEGIN() {

    // configure as though listing the test dir, since we are making a copy
    compare_visitor_init(&visitor, s_test_traverse_result, s_test_traverse_result_len);

    ERR_REGION_ERROR_CHECK(copy_dir(s_test_dir, s_copy_dir), err);

    traverse_dir_path(s_copy_dir, &visitor.handler_i);

    ERR_REGION_CMP_CHECK(visitor.line != s_test_traverse_result_len, err);

  } ERR_REGION_END()

  delete_dir(s_copy_dir);

  printf("%s\n", err ? "FAILED!" : "passed.");

  return err;
}