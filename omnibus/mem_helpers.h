#pragma once

#define SAFE_FREE(m) mh_safe_free((void**)(&m))

void mh_safe_free(void **m);
