#include "all_tests.h"

#include <stdio.h>
#include <string.h>

#include "filesystem.h"
#include "directory_traversal.h"
#include "directory_traversal_handler.h"

const char s_test_dir[] = "..\\test_data\\test_dir";

typedef struct dir_entry_fields {
  char const *name;
  short is_dir;
} dir_entry_fields_t;

static const dir_entry_fields_t s_test_list_dir_result[] = {
  {".", 1},
  {"..", 1},
  {"dir01", 1},
  {"dir02", 1},
  {"file01", 0},
  {"file02", 0},
};

static const size_t s_test_list_dir_result_len =
  sizeof(s_test_list_dir_result) / sizeof(*s_test_list_dir_result);

static const dir_entry_fields_t s_test_traverse_result[] = {
  {".", 1},
  {"..", 1},
  {"dir01", 1},
  {".", 1},
  {"..", 1},
  {"file0101", 0},
  {"file0102", 0},
  {"dir02", 1},
  {".", 1},
  {"..", 1},
  {"file0201", 0},
  {"file0202", 0},
  {"file01", 0},
  {"file02", 0},
};

static const size_t s_test_traverse_result_len =
  sizeof(s_test_traverse_result) / sizeof(*s_test_traverse_result);

//
// print visitor
//

typedef struct print_visiter {
  directory_traveral_handler_i handler_i;
} print_visitor_t;

static short pv_visit(struct directory_traveral_handler* self, struct file_handle* directory, struct directory_entry* entry) {
  printf("%s\\%s%s\n", directory->get_path(directory), 
    entry->get_name(entry), 
    entry->is_directory(entry) ? " [DIR]" : "");
  return 1;
}

static void pv_init_visitor(print_visitor_t* self) {
  self->handler_i.self = self;
  self->handler_i.visit = pv_visit;
}

//
// compare visitor
//

typedef struct compare_visiter {
  directory_traveral_handler_i handler_i;
  short passed;
  size_t line;
  dir_entry_fields_t const *result;
  size_t result_len;
} compare_visitor_t;

static short cv_visit(directory_traveral_handler_i* self_i, struct file_handle* directory, struct directory_entry* entry) {
  compare_visitor_t *self = (compare_visitor_t *)self_i->self;

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
// tests
//

void test_traverse_dirs(void) {
  compare_visitor_t visitor;
  cv_init_visitor(&visitor, s_test_traverse_result, s_test_traverse_result_len);

  printf("Checking traversal of directory %s... ", s_test_dir);

  traverse_dir_path(s_test_dir, &visitor.handler_i);

  if (visitor.line != s_test_traverse_result_len) visitor.passed = 0;

  printf("%s\n", visitor.passed ? "passed." : "FAILED!");
}

void test_list_dir(void) {
  size_t i = 0;
  short passed = 1;

  printf("Checking listing of directory %s... ", s_test_dir);

  file_handle_i *dir = open_dir(s_test_dir);
  while (!dir->is_eof(dir)) {
    if (i >= s_test_list_dir_result_len) {
      passed = 0;
      break;
    }

    directory_entry_i *entry = dir->next_dir_entry(dir);
    if (entry) {
      if (strcmp(s_test_list_dir_result[i].name, entry->get_name(entry)) != 0) {
        passed = 0;
        break;
      }

      if (s_test_list_dir_result[i].is_dir != entry->is_directory(entry)) {
        passed = 0;
        break;
      }

      entry->release(entry);
      ++i;
    }
  }

  dir->close(dir);

  if (i != s_test_list_dir_result_len) passed = 0;

  printf("%s\n", passed ? "passed." : "FAILED!");
}
