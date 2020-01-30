#pragma once

#include <stddef.h>

errno_t test_string_join(void);
errno_t test_getline(void);
errno_t test_cue(void);
errno_t test_cue_copy(void);
errno_t test_cue_transform(void);
errno_t test_cue_traverse(void);
errno_t test_list_dir(void);
errno_t test_traverse_dirs(void);
errno_t test_enumerate_path(void);
errno_t test_ensure_path(void);
errno_t test_string_stack(void);
errno_t test_string_unstack(void);
errno_t test_string_queue(void);
errno_t test_string_vector_join(void);
errno_t test_string_holder_char(void);
errno_t test_string_holder_str(void);
errno_t test_int_queue(void);
errno_t test_double_queue(void);
errno_t test_parallel_traverse(void);

