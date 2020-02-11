#pragma once

#include <stddef.h>

struct line_reader;
struct line_writer;

errno_t read_write_all_lines(struct line_reader *reader, struct line_writer *writer);
