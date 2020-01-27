#include "directory_traversal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "filesystem.h"
#include "directory_traversal_handler.h"

static const char s_current_dir[] = ".";
static const char s_parent_dir[] = "..";

short traverse_dir_path_opts(char const* path, struct directory_traversal_options const* opts, struct directory_traveral_handler* handler) {
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

short traverse_dir_path(char const* path, struct directory_traveral_handler *handler) {
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

short traverse_dir_opts(file_handle_i* directory, struct directory_traversal_options const *opts, struct directory_traveral_handler* handler) {
  short keep_traversing = 1;
  file_handle_i* dir = directory;

  directory_traversal_handler_state_t state;
  memset(&state, 0, sizeof(state));
  state.directory = dir;
  state.first_entry = 1;

  while (!dir->is_eof(dir) && keep_traversing) {

    directory_entry_i* entry = dir->next_dir_entry(dir);
    if (entry) {
      if (!should_visit(entry)) {
        entry->release(entry);
        continue;
      }

      state.entry = entry;
      state.last_entry = dir->is_eof(dir);

      if (! opts->post_visit) keep_traversing = handler->visit(handler, &state);

      if (should_traverse(entry) && keep_traversing && opts->should_descend) {
        file_handle_i* subdir = dir->open_directory(dir, entry->get_name(entry));
        if (subdir) {
          keep_traversing = traverse_dir_opts(subdir, opts, handler);
          subdir->close(subdir);
        }
      }

      if (opts->post_visit) keep_traversing = handler->visit(handler, &state);

      entry->release(entry);
      state.first_entry = 0;
    }
  }

  return keep_traversing;
}

short traverse_dir(file_handle_i *directory, struct directory_traveral_handler* handler) {
  directory_traversal_options_t opts;
  memset(&opts, 0, sizeof(opts));
  opts.should_descend = 1;

  return traverse_dir_opts(directory, &opts, handler);
}
