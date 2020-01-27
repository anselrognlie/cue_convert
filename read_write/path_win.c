#include "path_shared.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include "mem_helpers.h"

errno_t ps_path_enumerator_init(struct ps_path_enumerator* self, const char* path);
errno_t ps_start_path_enumeration(ps_path_enumerator_t* self);

static const char s_path_separator = '\\';
static const char s_drive_separator = ':';
static char const s_current_dir[] = ".";
static const size_t s_current_dir_len = sizeof(s_current_dir) / sizeof(*s_current_dir) - 1;
static char const s_parent_dir[] = "..";
static const size_t s_parent_dir_len = sizeof(s_parent_dir) / sizeof(*s_parent_dir) - 1;

struct fs_path_enumerator_rules;

typedef struct fs_path_enumerator_rules {
  path_enumerator_rules_i rules_i;
} fs_path_enumerator_rules_t;

typedef struct fs_path_enumerator {
  ps_path_enumerator_t shared;
} fs_path_enumerator_t;

static short fs_should_skip_path(struct path_enumerator_rules* self, path_enumerate_status_t const* status);
static void fs_rule_dispose(struct path_enumerator_rules* self);

static path_enumerator_rules_i *fs_path_enumerator_rules_alloc() {
  fs_path_enumerator_rules_t *rules = malloc(sizeof(fs_path_enumerator_rules_t));
  if (! rules) return NULL;

  memset(rules, 0, sizeof(fs_path_enumerator_rules_t));

  rules->rules_i.self = rules;
  rules->rules_i.dispose = fs_rule_dispose;
  rules->rules_i.should_skip_path = fs_should_skip_path;

  return &rules->rules_i;
}

static short fs_should_skip_path(struct path_enumerator_rules* self, path_enumerate_status_t const* status) {
  // check whether the last char of the path is a drive separator, or a current or parent dir
  short is_drive = *(status->path_end - 1) == s_drive_separator;
  short is_dot = ((status->path_end - status->path_start) == s_current_dir_len) &&
    (memcmp(status->path_start, s_current_dir, s_current_dir_len) == 0);
  short is_dot_dot = ((status->path_end - status->path_start) == s_parent_dir_len) &&
    (memcmp(status->path_start, s_parent_dir, s_parent_dir_len) == 0);
  
  return is_drive || is_dot || is_dot_dot;
}

static void fs_rule_dispose(struct path_enumerator_rules* self_i) {
  fs_path_enumerator_rules_t *self = (fs_path_enumerator_rules_t *)self_i;
  free(self);
}

path_enumerator_i* enumerate_path(char const* path) {
  fs_path_enumerator_t *enumerator = malloc(sizeof(*enumerator));
  if (! enumerator) return NULL;

  if (ps_path_enumerator_init(&enumerator->shared, path)) {
    free(enumerator);
    return NULL;
  }

  enumerator->shared.enum_i.self = enumerator;
  enumerator->shared.path_separator = '\\';
  enumerator->shared.rules = fs_path_enumerator_rules_alloc();

  ps_start_path_enumeration(&enumerator->shared);

  return &enumerator->shared.enum_i;
}

//errno_t ensure_path(char const* path) {
//  // walk the path breaking at directory separators and make sure 
//  // the entire set of directories exists
//
//  wchar_t* path_w = widen_path(path);
//  if (!path_w) return -1;
//
//  size_t path_len = wcslen(path_w);
//  if (!path_len) {
//    SAFE_FREE(path_w);
//    return -1;
//  }
//
//  wchar_t* path_start = path_w,
//    * path_end = path_w,
//    * last_path_end = path_w;
//  short done = 0;
//
//  // find next delim
//  while (!done) {
//    path_end = wcschr(path_start, s_path_separator_w);
//    if (!path_end) {
//      path_end = path_w + path_len;
//      done = 1;
//    }
//
//    // check whether previous char was a : (part of c:\...)
//    if (*(path_end - 1) == s_drive_separator_w) {
//      // just move on
//      last_path_end = path_end;
//      path_start = path_end + 1;
//      continue;
//    }
//
//    // check whether the path is either . or ..
//    short is_dot = ((path_end - path_start) == s_current_dir_len) &&
//      (memcmp(path_start, s_current_dir, s_current_dir_len) == 0);
//    short is_dot_dot = ((path_end - path_start) == s_parent_dir_len) &&
//      (memcmp(path_start, s_parent_dir, s_parent_dir_len) == 0);
//    if (is_dot || is_dot_dot) {
//      // just move on
//      last_path_end = path_end;
//      path_start = path_end + 1;
//      continue;
//    }
//
//    // ok, we have a chunk of path
//    *path_end = 0;
//    printf("%ls\n", path_w);
//    //BOOL result = Create
//    *path_end = s_path_separator_w;
//
//    // update pointers
//    last_path_end = path_end;
//    path_start = path_end + 1;
//  }
//
//  SAFE_FREE(path_w);
//
//  return 0;
//}

#endif
