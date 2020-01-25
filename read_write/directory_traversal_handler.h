#pragma once

struct directory_traveral_handler;

typedef struct directory_traveral_handler {
  void* self;
  size_t(*visit)(struct directory_traveral_handler* self, char const** line_out, size_t* bytes_out);
} directory_traveral_handler_i;