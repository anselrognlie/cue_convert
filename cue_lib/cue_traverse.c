#include "cue_traverse.h"

#include <stdio.h>

#include "err_helpers.h"
#include "filesystem.h"
#include "string_vector.h"
#include "char_vector.h"
#include "string_helpers.h"
#include "mem_helpers.h"

//
// cue traversal visitor
//

static short ctv_visit(directory_traversal_handler_i* self_i, directory_traversal_handler_state_t const* state) {
  cue_traverse_visitor_t* self = (cue_traverse_visitor_t*)self_i->self;
  file_handle_i const* directory = state->directory;
  directory_entry_i const* entry = state->entry;
  string_vector_t* history = state->history;

  short keep_traversing = 1;
  char const* path = 0;
  char const* source_path = 0;
  char const* target_path = 0;
  char const* parallel_path = 0;

  ERR_REGION_BEGIN() {

    path = join_strings(
      string_vector_get_buffer(history),
      string_vector_get_length(history),
      "\\");
    ERR_REGION_NULL_CHECK_CODE(path, keep_traversing, 0);

    target_path = char_vector_get_str(self->target_path);
    ERR_REGION_NULL_CHECK_CODE(target_path, keep_traversing, 0);

    char const* parts[] = { target_path, path };
    parallel_path = join_strings(parts, 2, k_path_separator);
    ERR_REGION_NULL_CHECK_CODE(parallel_path, keep_traversing, 0);

    printf("ctv:%s\n", parallel_path);

  } ERR_REGION_END()

  SAFE_FREE(parallel_path);
  SAFE_FREE(target_path);
  SAFE_FREE(path);

  return keep_traversing;
}

errno_t ctv_init_visitor(cue_traverse_visitor_t* self,
  char const* target_path,
  char const* source_path,
  short generate_report,
  short execute) {

  errno_t err = 0;

  char_vector_t* target_path_str = 0;
  char_vector_t* source_path_str = 0;

  ERR_REGION_BEGIN() {
    memset(self, 0, sizeof(*self));
    self->handler_i.self = self;
    self->handler_i.visit = ctv_visit;
    self->generate_report = generate_report;
    self->execute = execute;

    target_path_str = char_vector_alloc();
    ERR_REGION_NULL_CHECK(target_path_str, err);
    ERR_REGION_NULL_CHECK(char_vector_set_str(target_path_str, target_path), err);
    self->target_path = target_path_str;

    source_path_str = char_vector_alloc();
    ERR_REGION_NULL_CHECK(source_path_str, err);
    ERR_REGION_NULL_CHECK(char_vector_set_str(source_path_str, source_path), err);
    self->source_path = source_path_str;

    return err;

  } ERR_REGION_END()

  if (source_path_str) char_vector_free(source_path_str);
  if (target_path_str) char_vector_free(target_path_str);

  return err;
}

errno_t ctv_uninit_visitor(cue_traverse_visitor_t* self) {
  errno_t err = 0;
  errno_t op_err = 0;
  if (self->source_path) op_err = char_vector_free(self->source_path);
  err = err ? err : op_err;
  if (self->target_path) op_err = char_vector_free(self->target_path);
  err = err ? err : op_err;
  memset(self, 0, sizeof(*self));
  return err;
}

