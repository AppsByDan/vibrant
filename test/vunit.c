#include "vunit.h"

#ifdef __cplusplus
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#else
#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

// ////////////////////////////////////
// private types
// ////////////////////////////////////

typedef enum vu_case_state_t {
  VU__CASE_STATE_NONE,
  VU__CASE_STATE_RUNNING,
  VU__CASE_STATE_FAILED,
} vu_case_state_t;

struct vu_context_t {
  char active_case_name[41];
  vu_case_state_t active_case_state;
  int failed_cases;
  int total_cases;
  FILE* out;
};

// ////////////////////////////////////
// private functions
// ////////////////////////////////////

static void vu_context_resolve_active_case(vu_context_t* context) {
  if (context->active_case_state == VU__CASE_STATE_NONE) {
    fprintf(context->out, "FATAL: no active CASE to resolve\n");
    abort();
  }

  if (context->active_case_state == VU__CASE_STATE_RUNNING) {
    fprintf(context->out, "PASS\n");
  } else if (context->active_case_state == VU__CASE_STATE_FAILED) {
    // "FAIL" is printed in vu__context_report_case()
    context->failed_cases++;
  }

  context->active_case_name[0] = '\0';
  context->active_case_state = VU__CASE_STATE_NONE;
}

static float next_4_ulp_float(float x) {
  for (int i = 0; i < 4; i++) {
    x = nextafterf(x, FLT_MAX);
  }
  return x;
}

static float prev_4_ulp_float(float x) {
  for (int i = 0; i < 4; i++) {
    x = nextafterf(x, -FLT_MAX);
  }
  return x;
}

static double next_4_ulp_double(double x) {
  for (int i = 0; i < 4; i++) {
    x = nextafter(x, DBL_MAX);
  }
  return x;
}

static double prev_4_ulp_double(double x) {
  for (int i = 0; i < 4; i++) {
    x = nextafter(x, -DBL_MAX);
  }
  return x;
}

// ////////////////////////////////////
// public functions
// ////////////////////////////////////

// float compare from: https://github.com/Warwolt/rktest (rktest.h)

bool vu__floats_within_4_ulp(float lhs, float rhs) {
  return prev_4_ulp_float(rhs) <= lhs && lhs <= next_4_ulp_float(rhs);
}

bool vu__doubles_within_4_ulp(double lhs, double rhs) {
  return prev_4_ulp_double(rhs) <= lhs && lhs <= next_4_ulp_double(rhs);
}

void vu__printf(vu_context_t* context, const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(context->out, format, args);
  va_end(args);
}

void vu__context_create_case(vu_context_t* context, const char* name) {
  if (context->active_case_state != VU__CASE_STATE_NONE) {
    vu_context_resolve_active_case(context);
  }

  // copy name so we dont have to worry about pointer lifetimes

  const size_t max = vu_arr_len(context->active_case_name) - 1;
  const size_t len = strlen(name);
  const size_t case_name_len = len < max ? len : max;

  memcpy(context->active_case_name, name, case_name_len);
  context->active_case_name[case_name_len] = '\0';

  context->active_case_state = VU__CASE_STATE_RUNNING;
  context->total_cases++;

  fprintf(context->out, ":: %-40s ", context->active_case_name);
}

void vu__context_report_case(vu_context_t* context, bool passed) {
  if (context->active_case_state == VU__CASE_STATE_NONE) {
    // CASE has not been defined in the test, but an ASSERT has been called.
    // instead of aborting, create a case with an ugly name.
    vu__context_create_case(context, "!!! NO ACTIVE CASE !!!");
  }

  if (!passed && context->active_case_state == VU__CASE_STATE_RUNNING) {
    context->active_case_state = VU__CASE_STATE_FAILED;
    fprintf(context->out, "FAIL");
  }
}

int vu_run(const vu_test_id_t* test_list, size_t count) {
  FILE* out = stdout;
  int failed_tests = 0;
  int total_failed_cases = 0;
  int total_cases = 0;

  for (size_t i = 0; i < count; ++i) {
    fprintf(out, "%s\n", test_list[i].name);

    vu_context_t context;

    memset(&context, 0, sizeof(context));
    context.out = out;

    test_list[i].fn(&context);

    // Report the last case
    if (context.active_case_state != VU__CASE_STATE_NONE) {
      vu_context_resolve_active_case(&context);
    }

    if (context.failed_cases > 0) {
      failed_tests++;
    }
    total_failed_cases += context.failed_cases;
    total_cases += context.total_cases;
    fprintf(out, "%d cases, %d failed\n", context.total_cases,
            context.failed_cases);
    fprintf(out, "\n");
  }

  fprintf(out, "Summary:\n");
  fprintf(out, "  %d tests run\n", (int)count);
  fprintf(out, "  %d tests failed\n", failed_tests);
  fprintf(out, "  %d cases failed\n", total_failed_cases);
  fprintf(out, "  %d cases passed\n", total_cases - total_failed_cases);

  return failed_tests;
}
