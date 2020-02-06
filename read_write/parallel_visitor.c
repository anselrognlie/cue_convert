#include "parallel_visitor.h"

#include "filesystem.h"
#include "string_vector.h"
#include "err_helpers.h"
#include "mem_helpers.h"
#include "string_helpers.h"

static short parallel_visitor_handle(
  directory_traversal_handler_i* self_i, 
  directory_traversal_handler_state_t const* state,
  parallel_visitor_callback callback) {

  // do nothing if no callback
  if (! callback) { return 1; }

  parallel_visitor_t* self = (parallel_visitor_t*)self_i->self;
  file_handle_i const* directory = state->directory;
  directory_entry_i const* entry = state->entry;
  string_vector_t* history = state->history;

  short keep_traversing = 1;
  char const* path = 0;
  char const* parallel_path = 0;

  ERR_REGION_BEGIN() {
    path = join_cstrs(
      history->get_buffer(history),
      history->get_length(history),
      k_path_separator);
    ERR_REGION_NULL_CHECK_CODE(path, keep_traversing, 0);

    char const* parts[] = { self->root_path, path };
    parallel_path = join_cstrs(parts, 2, k_path_separator);
    ERR_REGION_NULL_CHECK_CODE(parallel_path, keep_traversing, 0);

    parallel_visitor_state_t p_state = { 0 };
    p_state.base_state = state;
    p_state.parallel_path = parallel_path;

    keep_traversing = callback(self, &p_state);

  } ERR_REGION_END()

  SAFE_FREE(parallel_path);
  SAFE_FREE(path);

  return keep_traversing;
}

static short parallel_visitor_visit(directory_traversal_handler_i* self_i, directory_traversal_handler_state_t const* state) {
  parallel_visitor_t *self = (parallel_visitor_t*)self_i->self;
  if (self->visit) {
    return parallel_visitor_handle(self_i, state, self->visit);
  }

  return 1;
}

static short parallel_visitor_exit(directory_traversal_handler_i* self_i, directory_traversal_handler_state_t const* state) {
  parallel_visitor_t* self = (parallel_visitor_t*)self_i->self;
  if (self->exit) {
    return parallel_visitor_handle(self_i, state, self->exit);
  }

  return 1;
}

errno_t parallel_visitor_init(parallel_visitor_t* self,
  char const* root_path) {

  errno_t err = 0;

  ERR_REGION_BEGIN() {
    memset(self, 0, sizeof(*self));
    self->handler_i.self = self;
    self->handler_i.visit = parallel_visitor_visit;
    self->handler_i.exit = parallel_visitor_exit;

    ERR_REGION_NULL_CHECK(self->root_path = _strdup(root_path), err);

  } ERR_REGION_END()

  return err;
}

void parallel_visitor_uninit(parallel_visitor_t* self) {
  SAFE_FREE(self->root_path);
}
