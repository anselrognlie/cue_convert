#pragma once

#define ERR_REGION_BEGIN() do
#define ERR_REGION_END() while(0);
#define ERR_EXIT() break;

#define ERR_REGION_NULL_CHECK(arg, err) \
  ERR_REGION_NULL_CHECK_CODE(arg, err, -1)

#define ERR_REGION_NULL_CHECK_CODE(arg, err, code) \
  if (!(arg)) { \
    err = (code); \
    ERR_EXIT() \
  }

#define ERR_REGION_CMP_CHECK(cmp, err) \
  ERR_REGION_CMP_CHECK_CODE(cmp, err, -1)

#define ERR_REGION_CMP_CHECK_CODE(cmp, err, code) \
  if ((cmp)) { \
    err = (code); \
    ERR_EXIT() \
  }

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define ERR_REGION_INVALID_CHECK(arg, err) \
  ERR_REGION_INVALID_CHECK_CODE(arg, err, -1)

#define ERR_REGION_INVALID_CHECK_CODE(arg, err, code) \
  if (INVALID_HANDLE_VALUE == (arg)) { \
    err = (code); \
    ERR_EXIT() \
  }

#endif