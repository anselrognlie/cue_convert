#pragma once

struct path_enumerator;

typedef struct path_enumerate_status {
  char const* path_start;
  char const* path_end;
  char const* full_path_start;
} path_enumerate_status_t;

typedef struct path_enumerator_rules {
  void* self;
  short (*should_skip_path)(struct path_enumerator_rules* self, path_enumerate_status_t const* status);
  void (*dispose)(struct path_enumerator_rules* self);
} path_enumerator_rules_i;

typedef struct path_enumerator {
  void* self;
  short (*has_next)(struct path_enumerator* self);
  void (*dispose)(struct path_enumerator* self);
  path_enumerate_status_t(*next)(struct path_enumerator* self);
} path_enumerator_i;

path_enumerator_i* enumerate_path(char const* path);
char const* path_dir_part(char const* path);
char const* path_file_part(char const* path);
char const* join_dir_file_path(char const* dir, char const* file);
char const* join_path_parts(char const** parts);

char const* file_name_part(char const* filename);
char const* file_ext_part(char const* filename);

