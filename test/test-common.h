#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include "vunit.h"

#ifdef __cplusplus
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
extern "C" {
#else
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#ifdef __cplusplus
}
#endif

#include <vibrant.h>

// define the correct infinity, based on vibrant input precision mode
#if defined(VIBRANT_DOUBLE_PRECISION)
#define INF HUGE_VAL
#else
#define INF HUGE_VALF
#endif

// asserts that a vbt_recv_t object contains the expect rgba u8 values
#define ASSERT_RECV_U8(RESULT, RECV, EXP_R, EXP_G, EXP_B, EXP_A) \
  do {                                                           \
    ASSERT_EQ(RESULT, VBT_SUCCESS);                              \
    vbt_recv_t recv2 = RECV;                                     \
    ASSERT_EQ(recv2.tag, VBT_RECV_VAL_U8);                       \
    ASSERT_EQ(recv2.u.val.u8.r, EXP_R);                          \
    ASSERT_EQ(recv2.u.val.u8.g, EXP_G);                          \
    ASSERT_EQ(recv2.u.val.u8.b, EXP_B);                          \
    ASSERT_EQ(recv2.u.val.u8.a, EXP_A);                          \
  } while (0)

#endif  // TEST_COMMON_H
