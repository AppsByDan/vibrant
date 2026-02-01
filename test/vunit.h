#ifndef VUNIT_H
#define VUNIT_H

// vunit
//
// A simple unit test library that works with C and C++.
//
// Test function signatures are defined with the TEST macro. Within
// a test function, named sections are defined with CASE(name). Within
// a CASE, ASSERT macros can be called.
//
// TEST(my_test) {
//   CASE("my test case") {
//     ASSERT_EQ(1, 1);
//   }
// }
//
// The runner takes a list of test functions and names and runs the test,
// displaying test names, case descriptions and PASS/FAIL info. The asserts
// will print diffs on error. All prints go to stdout. The runner does not
// stop for assertions. The runner and vunit are single threaded.
//
// There is no automatic test discovery. For vibrant, I made a cmake script
// that finds TEST macros in test files and sets up a unity build of the
// test runner.
//
// main () {
//   vu_test_id_t tests[] = {
//     TESTID(my_test),
//   };
//
//   return vu_run(tests, vu_arr_len(tests));
// }

#if defined __GNUC__
#define SDLI_GNUATTR(...) __attribute__((__VA_ARGS__))
#else
#define SDLI_GNUATTR(...)
#endif

#ifdef __cplusplus
#include <cstddef>
#else
#include <stdbool.h>
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ////////////////////////////////////
// types
// ////////////////////////////////////

typedef struct vu_context_t vu_context_t;
typedef void (*vu_test_fn)(vu_context_t*);

typedef struct {
  const char* name;
  vu_test_fn fn;
} vu_test_id_t;

// ////////////////////////////////////
// public functions
// ////////////////////////////////////

// run a list of test methods and report to stdout
int vu_run(const vu_test_id_t* test_list, size_t count);

// ////////////////////////////////////
// macro only functions
// ////////////////////////////////////

// create a new case in the current text context
void vu__context_create_case(vu_context_t* context, const char* name);
// update the active case with an assertion result
void vu__context_report_case(vu_context_t* context, bool passed);
// print to vunits output target (stdout)
void vu__printf(vu_context_t* context, const char* format, ...)
    SDLI_GNUATTR(format(printf, 2, 3));
bool vu__floats_within_4_ulp(float lhs, float rhs);
bool vu__doubles_within_4_ulp(double lhs, double rhs);

#ifdef __cplusplus
}
#endif

// ////////////////////////////////////
// macros
// ////////////////////////////////////

// test function signature
#define TEST(name) static void test_##name(vu_context_t* context)

// create a new case within a test function
#define CASE(name) if (vu__context_create_case(context, name), 1)

// define a testid array element
#define TESTID(name) {"test_" #name, &test_##name}

// assert ints are equal
#define ASSERT_EQ(a, b)                                         \
  do {                                                          \
    int val_a = (a);                                            \
    int val_b = (b);                                            \
    bool passed = (val_a == val_b);                             \
    vu__context_report_case(context, passed);                   \
    if (!passed) {                                              \
      vu__printf(context, "   ASSERT_EQ(%s, %s)\n", #a, #b);    \
      vu__printf(context, "   at %s:%d\n", __FILE__, __LINE__); \
      vu__printf(context, "     expected: %d\n", val_b);        \
      vu__printf(context, "     actual:   %d\n", val_a);        \
    }                                                           \
  } while (0)

// assert floats are equal enough
#define ASSERT_FLOAT_EQ(a, b)                                      \
  do {                                                             \
    float val_a = (a);                                             \
    float val_b = (b);                                             \
    bool passed = vu__floats_within_4_ulp(val_a, val_b);           \
    vu__context_report_case(context, passed);                      \
    if (!passed) {                                                 \
      vu__printf(context, "   ASSERT_FLOAT_EQ(%s, %s)\n", #a, #b); \
      vu__printf(context, "   at %s:%d\n", __FILE__, __LINE__);    \
      vu__printf(context, "     expected: %f\n", val_b);           \
      vu__printf(context, "     actual:   %f\n", val_a);           \
    }                                                              \
  } while (0)

// assert doubles are equal enough
#define ASSERT_DOUBLE_EQ(a, b)                                      \
  do {                                                              \
    double val_a = (a);                                             \
    double val_b = (b);                                             \
    bool passed = vu__doubles_within_4_ulp(val_a, val_b);           \
    vu__context_report_case(context, passed);                       \
    if (!passed) {                                                  \
      vu__printf(context, "   ASSERT_DOUBLE_EQ(%s, %s)\n", #a, #b); \
      vu__printf(context, "   at %s:%d\n", __FILE__, __LINE__);     \
      vu__printf(context, "     expected: %f\n", val_b);            \
      vu__printf(context, "     actual:   %f\n", val_a);            \
    }                                                               \
  } while (0)

// len of a non-decayed array
#define vu_arr_len(arr) (sizeof(arr) / sizeof((arr)[0]))

#endif  // VUNIT_H
