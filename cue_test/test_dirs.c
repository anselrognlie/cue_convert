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

static const char s_test_dir[] = "..\\test_data\\test_dir";
static const char s_test_delete_dir[] = "..\\test_data\\ensure_dir";
static const char s_test_ensure_dir[] = "..\\test_data\\ensure_dir\\a\\dir\\to\\ensure";
static const char s_parallel_dir[] = "p:\\parallel_path";

//#define PRINT_ONLY

typedef struct dir_entry_fields {
  char const *name;
  short is_dir;
} dir_entry_fields_t;

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
// print visitor
//

typedef struct print_visitor {
  directory_traversal_handler_i handler_i;
} print_visitor_t;

static short pv_visit(struct directory_traversal_handler* self, directory_traversal_handler_state_t const *state) {
  file_handle_i const *directory = state->directory;
  directory_entry_i const *entry = state->entry;
  printf("%s\\%s%s%s%s\n", directory->get_path(directory), 
    entry->get_name(entry), 
    entry->is_directory(entry) ? " [DIR]" : "",
    state->first_entry ? " [FIRST]" : "",
    state->last_entry ? " [LAST]" : ""
  );
  return 1;
}

static void pv_init_visitor(print_visitor_t* self) {
  memset(self, 0, sizeof(*self));
  self->handler_i.self = self;
  self->handler_i.visit = pv_visit;
}

//
// compare visitor
//

typedef struct compare_visitor {
  directory_traversal_handler_i handler_i;
  short passed;
  size_t line;
  dir_entry_fields_t const *result;
  size_t result_len;
} compare_visitor_t;

static short cv_visit(directory_traversal_handler_i* self_i, directory_traversal_handler_state_t const* state) {
  compare_visitor_t *self = (compare_visitor_t *)self_i->self;
  file_handle_i const* directory = state->directory;
  directory_entry_i const* entry = state->entry;

  size_t i = self->line;
  if (i >= self->result_len) {
    self->passed = 0;
  }

  if (strcmp(self->result[i].name, entry->get_name(entry)) != 0) {
    self->passed = 0;
  }

  if (self->result[i].is_dir != entry->is_directory(entry)) {
    self->passed = 0;
  }

  ++self->line;

  return self->passed;
}

static void cv_init_visitor(compare_visitor_t* self,
  dir_entry_fields_t const* result,
  size_t result_len) {

  memset(self, 0, sizeof(*self));
  self->handler_i.self = self;
  self->handler_i.visit = cv_visit;
  self->passed = 1;
  self->result = result;
  self->result_len = result_len;
}

//
// parallel traversal visitor
//

typedef struct parallel_traverse_visitor {
  directory_traversal_handler_i handler_i;
  char_vector_t root_path;
  short passed;
  size_t line;
  char const** result;
  size_t result_len;
} parallel_traverse_visitor_t;

static short ptv_visit_exit(directory_traversal_handler_i* self_i, directory_traversal_handler_state_t const* state) {
  parallel_traverse_visitor_t* self = (parallel_traverse_visitor_t*)self_i->self;
  file_handle_i const* directory = state->directory;
  directory_entry_i const* entry = state->entry;
  string_vector_t* history = state->history;

  short keep_traversing = 1;
  char const* path = 0;
  char const* root_path = 0;
  char const* parallel_path = 0;

  ERR_REGION_BEGIN() {
    size_t i = self->line;
    ERR_REGION_CMP_CHECK_CODE(i >= self->result_len, keep_traversing, 0);

    path = join_strings(
      string_vector_get_buffer(history),
      string_vector_get_length(history),
      "\\");
    ERR_REGION_NULL_CHECK_CODE(path, keep_traversing, 0);

    root_path = char_vector_get_str(&self->root_path);
    ERR_REGION_NULL_CHECK_CODE(root_path, keep_traversing, 0);

    char const* parts[] = { root_path, path };
    parallel_path = join_strings(parts, 2, "\\");
    ERR_REGION_NULL_CHECK_CODE(parallel_path, keep_traversing, 0);

    ERR_REGION_CMP_CHECK_CODE(strcmp(self->result[i], parallel_path) != 0, keep_traversing, 0);
  } ERR_REGION_END()

  SAFE_FREE(parallel_path);
  SAFE_FREE(root_path);
  SAFE_FREE(path);

  if (self->passed) self->passed = keep_traversing;
  ++self->line;

  return keep_traversing;
}

static short ptv_visit(directory_traversal_handler_i* self_i, directory_traversal_handler_state_t const* state) {
  return ptv_visit_exit(self_i, state);
}

static errno_t ptv_init_visitor(parallel_traverse_visitor_t* self,
  char const *root_path,
  char const **result,
  size_t result_len) {

  errno_t err = 0;

  ERR_REGION_BEGIN() {
    memset(self, 0, sizeof(*self));
    self->handler_i.self = self;
    self->handler_i.visit = ptv_visit;
    self->passed = 1;
    self->result = result;
    self->result_len = result_len;

    ERR_REGION_ERROR_CHECK(char_vector_init(&self->root_path), err);
    ERR_REGION_NULL_CHECK(char_vector_set_str(&self->root_path, root_path), err);

  } ERR_REGION_END()

  return err;
}

static errno_t ptv_uninit_visitor(parallel_traverse_visitor_t* self) {
  return char_vector_uninit(&self->root_path);
}

//
// tests
//

void test_traverse_dirs_print(void) {
  print_visitor_t visitor;
  pv_init_visitor(&visitor);

  traverse_dir_path(s_test_dir, &visitor.handler_i);
}

errno_t test_traverse_dirs(void) {
#ifndef PRINT_ONLY
  compare_visitor_t visitor;
  cv_init_visitor(&visitor, s_test_traverse_result, s_test_traverse_result_len);

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
  pv_init_visitor(&visitor);

  directory_traversal_options_t opts;
  memset(&opts, 0, sizeof(opts));

  traverse_dir_path_opts(s_test_dir, &opts, &visitor.handler_i);
}

errno_t test_list_dir(void) {
#ifndef PRINT_ONLY
  compare_visitor_t visitor;
  cv_init_visitor(&visitor, s_test_list_dir_result, s_test_list_dir_result_len);

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
  cv_init_visitor(&visitor, s_test_ensure_result, s_test_ensure_result_len);

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
  ptv_init_visitor(&visitor, s_parallel_dir, s_parallel_dirs, result_len);

  traverse_dir_path(s_test_dir, &visitor.handler_i);

  ptv_uninit_visitor(&visitor);

  printf("%s\n", visitor.passed ? "passed." : "FAILED!");

  return visitor.passed ? 0 : -1;
}
