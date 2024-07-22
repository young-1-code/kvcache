#ifndef __COMMON_H__
#define __COMMON_H__

#include <assert.h>
#include <stdio.h>

#define kv_assert(exper) assert(exper)
// #define debug(format, ...) printf(format, __VA_ARGS__)
#define log(_fmt, ...) printf("[FILE:%s FUNC:%s LINE:%d] "_fmt, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define success  0
#define failed  -1

#endif