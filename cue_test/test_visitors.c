#include "test_visitors.h"

#include <stddef.h>
#include <stdio.h>

#include "err_helpers.h"
#include "filesystem.h"
#include "string_vector.h"
#include "char_vector.h"
#include "string_helpers.h"

//
// print visitor
//

static short pv_visit(struct directory_traversal_handler* self, directory_traversal_handler_state_t const* state) {
  file_handle_i const* directory = state->directory;
  directory_entry_i const* entry = state->entry;
  printf("%s\\%s%s%s%s\n", directory->get_path(directory),
    entry->get_name(entry),
    entry->is_directory(entry) ? " [DIR]" : "",
    state->first_entry ? " [FIRST]" : "",
    state->last_entry ? " [LAST]" : ""
  );
  return 1;
}

void print_visitor_init(print_visitor_t* self) {
  memset(self, 0, sizeof(*self));
  self->handler_i.self = self;
  self->handler_i.visit = pv_visit;
}

//
// compare visitor
//

static short cv_visit(directory_traversal_handler_i* self_i, directory_traversal_handler_state_t const* state) {
  compare_visitor_t* self = (compare_visitor_t*)self_i->self;
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

void compare_visitor_init(compare_visitor_t* self,
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

static short ptv_visit(parallel_visitor_t *self_t, parallel_visitor_state_t const* state) {
  parallel_traverse_visitor_t* self = (parallel_traverse_visitor_t*)self_t->self;

  short keep_traversing = 1;
  char const* parallel_path = state->parallel_path;

  ERR_REGION_BEGIN() {
    size_t i = self->line;
    ERR_REGION_CMP_CHECK_CODE(i >= self->result_len, keep_traversing, 0);

    ERR_REGION_CMP_CHECK_CODE(strcmp(self->result[i], parallel_path) != 0, keep_traversing, 0);
  } ERR_REGION_END()

  if (self->passed) self->passed = keep_traversing;
  ++self->line;

  return keep_traversing;
}

errno_t parallel_traverse_visitor_init(parallel_traverse_visitor_t* self,
  char const* root_path,
  char const** result,
  size_t result_len) {

  errno_t err = 0;

  ERR_REGION_BEGIN() {
    memset(self, 0, sizeof(*self));

    parallel_visitor_t *pv = &self->pv_t;
    ERR_REGION_ERROR_CHECK(parallel_visitor_init(pv, root_path), err);

    pv->self = self;
    pv->visit = ptv_visit;
    self->passed = 1;
    self->result = result;
    self->result_len = result_len;

  } ERR_REGION_END()

    return err;
}

void parallel_traverse_visitor_uninit(parallel_traverse_visitor_t* self) {
  parallel_visitor_uninit(&self->pv_t);
}
