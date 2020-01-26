#include "directory_traversal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "filesystem.h"
#include "directory_traversal_handler.h"

static const char s_current_dir[] = ".";
static const char s_parent_dir[] = "..";

void traverse_dir_path(char const* path, struct directory_traveral_handler *handler) {
  file_handle_i* dir = open_dir(path);
  if (dir) {
    traverse_dir(dir, handler);
  }

  dir->close(dir);
}

static short should_traverse(directory_entry_i* entry) {
  if (! entry->is_directory(entry)) return 0;

  char const *dir = entry->get_name(entry);
  if (strcmp(s_current_dir, dir) == 0) return 0;
  if (strcmp(s_parent_dir, dir) == 0) return 0;

  return 1;
}

short traverse_dir(file_handle_i *directory, struct directory_traveral_handler* handler) {
  short keep_traversing = 1;
  file_handle_i* dir = directory;
  while (!dir->is_eof(dir) && keep_traversing) {

    directory_entry_i* entry = dir->next_dir_entry(dir);
    if (entry) {
      keep_traversing = handler->visit(handler, dir, entry);

      if (should_traverse(entry) && keep_traversing) {
        file_handle_i *subdir = dir->open_directory(dir, entry->get_name(entry));
        if (subdir) {
          keep_traversing = traverse_dir(subdir, handler);
          subdir->close(subdir);
        }
      }

      entry->release(entry);
    }
  }

  return keep_traversing;
}
