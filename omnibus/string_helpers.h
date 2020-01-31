#pragma once

#include <stddef.h>

char *join_cstrs(char const *strings[], size_t num_strings, char const *delim);
short cstr_ends_with(char const *str, char const *ending);
