#include "directory_traversal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "filesystem.h"
#include "directory_traversal_handler.h"
#include "string_vector.h"
#include "err_helpers.h"
#include "mem_helpers.h"

static const char s_current_dir[] = ".";
static const char s_parent_dir[] = "..";

typedef struct directory_traversal_state {
  struct string_vector* history;
} directory_traversal_state_t;

static short traverse_dir_internal(
  file_handle_i* directory, 
  struct directory_traversal_options const* opts, 
  struct directory_traversal_state *state,
  struct directory_traversal_handler* handler);

short traverse_dir_path_opts(char const* path, struct directory_traversal_options const* opts, struct directory_traversal_handler* handler) {
  short result = 0;
  file_handle_i* dir = open_dir(path);
  if (dir) {
    result = traverse_dir_opts(dir, opts, handler);
    dir->close(dir);
  }
  else {
    result = 0;
  }

  return result;
}

short traverse_dir_path(char const* path, struct directory_traversal_handler *handler) {
  directory_traversal_options_t opts;
  memset(&opts, 0, sizeof(opts));
  opts.should_descend = 1;

  return traverse_dir_path_opts(path, &opts, handler);
}

static short should_visit(directory_entry_i const* entry) {
  char const* dir = entry->get_name(entry);
  if (strcmp(s_current_dir, dir) == 0) return 0;
  if (strcmp(s_parent_dir, dir) == 0) return 0;

  return 1;
}

static short should_traverse(directory_entry_i const* entry) {
  if (!entry->is_directory(entry)) return 0;

  return should_visit(entry);
}

short traverse_dir_opts(file_handle_i* directory, struct directory_traversal_options const *opts, struct directory_traversal_handler* handler) {

  directory_traversal_state_t state;
  memset(&state, 0, sizeof(state));
  state.history = string_vector_alloc();
  if (! state.history) return 0;

  short keep_traversing = traverse_dir_internal(directory, opts, &state, handler);

  SAFE_FREE_HANDLER(state.history, string_vector_free);

  return keep_traversing;
}

short traverse_dir(file_handle_i *directory, struct directory_traversal_handler* handler) {
  directory_traversal_options_t opts;
  memset(&opts, 0, sizeof(opts));
  opts.should_descend = 1;

  return traverse_dir_opts(directory, &opts, handler);
}

static short traverse_dir_internal(
  file_handle_i* directory,
  struct directory_traversal_options const* opts,
  struct directory_traversal_state* state,
  struct directory_traversal_handler* handler) {

  short keep_traversing = 1;
  file_handle_i* dir = directory;
  directory_entry_i* entry = 0;

  directory_traversal_handler_state_t handler_state;
  memset(&handler_state, 0, sizeof(handler_state));
  handler_state.directory = dir;
  handler_state.first_entry = 1;
  handler_state.history = state->history;

  while (!dir->is_eof(dir) && keep_traversing) {
    ERR_REGION_BEGIN() {
      entry = dir->next_dir_entry(dir);
      ERR_REGION_NULL_CHECK_CODE(entry, keep_traversing, 0);

      if (!should_visit(entry)) {
        ERR_EXIT()
      }

      handler_state.entry = entry;
      handler_state.last_entry = dir->is_eof(dir);
      ERR_REGION_NULL_CHECK_CODE(
        handler_state.history->push(handler_state.history, entry->get_name(entry)),
        keep_traversing, 0);

      if (!opts->post_visit && handler->visit) {
        keep_traversing = handler->visit(handler, &handler_state);
      }

      if (should_traverse(entry) && keep_traversing && opts->should_descend) {
        file_handle_i* subdir = dir->open_directory(dir, entry->get_name(entry));
        if (subdir) {
          keep_traversing = traverse_dir_internal(subdir, opts, state, handler);
          subdir->close(subdir);
        }
      }

      if (opts->post_visit && handler->visit && keep_traversing) {
        keep_traversing = handler->visit(handler, &handler_state);
      }

      if (handler->exit && keep_traversing) {
        keep_traversing = handler->exit(handler, &handler_state);
      }

      handler_state.history->pop(handler_state.history);
      handler_state.first_entry = 0;

    } ERR_REGION_END()

    if (entry) entry->release(entry);
  }

  return keep_traversing;
}
