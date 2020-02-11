#pragma once

short regex_matches(char const* pattern, char const* str);
short regex_matches_any(char const* const *patterns, int num_filters, char const* str);
