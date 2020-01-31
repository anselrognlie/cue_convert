#pragma once

#define SAFE_FREE(m) mh_safe_free((void**)(&m))
#define SAFE_FREE_HANDLER(m, free) do{if(m){free(m);m=0;}}while(0)

void mh_safe_free(void **m);
