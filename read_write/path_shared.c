#include "path_shared.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include "mem_helpers.h"

static char const s_current_dir[] = ".";
static const size_t s_current_dir_len = sizeof(s_current_dir) / sizeof(*s_current_dir) - 1;
static char const s_parent_dir[] = "..";
static const size_t s_parent_dir_len = sizeof(s_parent_dir) / sizeof(*s_parent_dir) - 1;

struct ps_path_enumerator;

static short ps_has_next(struct path_enumerator* self);
static void ps_dispose(struct path_enumerator* self);
static path_enumerate_status_t ps_next(struct path_enumerator* self);

static errno_t ps_next_path_enumeration(ps_path_enumerator_t* self);
static short ps_should_skip_path(ps_path_enumerator_t* self, path_enumerate_status_t const* status);

errno_t ps_path_enumerator_init(ps_path_enumerator_t* self, char const* path) {
  memset(self, 0, sizeof(*self));
  self->enum_i.self = self;
  self->enum_i.has_next = ps_has_next;
  self->enum_i.next = ps_next;
  self->enum_i.dispose = ps_dispose;

  char* path_dup = _strdup(path);
  if (!path_dup) return -1;

  self->path = path_dup;
  self->path_len = strlen(path_dup);
  self->path_start = self->path;
  self->path_end = self->path;

  return 0;
}

static short ps_has_next(struct path_enumerator* self_i) {
  ps_path_enumerator_t* self = (ps_path_enumerator_t*)self_i;
  return self->has_next;
}

static void ps_dispose(struct path_enumerator* self_i) {
  ps_path_enumerator_t* self = (ps_path_enumerator_t*)self_i;

  SAFE_FREE(self->path);
  if (self->rules) self->rules->dispose(self->rules);
  free(self);
}

static path_enumerate_status_t get_path_status(ps_path_enumerator_t* self) {
  path_enumerate_status_t status = {
    .path_start = self->path_start,
    .path_end = self->path_end,
    .full_path_start = self->path,
  };

  return status;
}

static path_enumerate_status_t ps_next(struct path_enumerator* self_i) {
  ps_path_enumerator_t *self = (ps_path_enumerator_t *)self_i;
  path_enumerate_status_t status = get_path_status(self);

  ps_next_path_enumeration(self);

  return status;
}

static short ps_should_skip_path(ps_path_enumerator_t* self, path_enumerate_status_t const *status) {
  short skip = 0;

  path_enumerator_rules_i *rules = self->rules;
  if (rules) {
    skip = rules->should_skip_path(rules, status);
  }

  return skip;
}

errno_t ps_start_path_enumeration(ps_path_enumerator_t* self) {
  // bail if the path is empty
  size_t len = self->path_len;
  if (self->path + len == self->path_end) {
    self->has_next = 0;
    return 0;
  }

  char *path_end = strchr(self->path_start, self->path_separator);
  if (!path_end) {
    path_end = self->path_start + len;
  }

  self->path_end = path_end;
  self->has_next = 1;

  path_enumerate_status_t status = get_path_status(self);
  if (ps_should_skip_path(self, &status)) {
    return ps_next_path_enumeration(self);
  }

  return 0;
}

errno_t ps_next_path_enumeration(ps_path_enumerator_t* self) {
  path_enumerate_status_t status;
  
  do {
    // bail if we reached the end
    size_t len = self->path_len;
    if (self->path + len == self->path_end) {
      self->has_next = 0;
      return 0;
    }

    // advance the starting location
    char* path_start = self->path_end + 1;
    if (self->path + len < path_start) {
      self->has_next = 0;
      return 0;
    }

    char* path_end = strchr(path_start, self->path_separator);
    if (!path_end) {
      path_end = self->path + len;
    }

    self->path_start = path_start;
    self->path_end = path_end;
    self->has_next = 1;

    path_enumerate_status_t status = get_path_status(self);

  } while (ps_should_skip_path(self, &status));

  return 0;
}

