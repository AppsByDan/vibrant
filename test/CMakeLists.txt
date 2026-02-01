cmake_minimum_required(VERSION 3.10)
project(vibrant C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_CXX_STANDARD 11)
set(CMAKR_CXX_EXTENSIONS False)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# test executable setup
function(add_test_exe TARGET_NAME SOURCE_FILE COMPILE_FLAG)
  add_executable(${TARGET_NAME} "${SOURCE_FILE}")
  target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
  if (COMPILE_FLAG)
  target_compile_definitions(${TARGET_NAME} PRIVATE "-D${COMPILE_FLAG}")
  endif()
  target_link_libraries(${TARGET_NAME} PRIVATE vunit vibrant)
  target_compile_options(${TARGET_NAME} PUBLIC
    $<$<C_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
  )
endfunction()

# find TEST() macros in TEST_FILENAMES to build a TESTID() list. then,
# configure vunit-test-runner.c.in template with that information.
function (configure_test_runner TEST_FILENAMES SOURCE_FILENAME)
  set(TESTIDS "")

  foreach(TEST_SOURCE ${TEST_FILENAMES})
      file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_SOURCE}" TEST_LINES REGEX "^TEST\\(")
      foreach(TEST_LINE ${TEST_LINES})
          string(REGEX MATCH "TEST\\((.*)\\)" _ DUMMY "${TEST_LINE}")
          if(CMAKE_MATCH_1)
              set(TEST_NAME "${CMAKE_MATCH_1}")
              set(TESTID_ELEM "    TESTID(${TEST_NAME}),")
              set(TESTIDS "${TESTIDS}\n${TESTID_ELEM}")
          endif()
      endforeach()
  endforeach()

  foreach(TEST_SOURCE ${TEST_FILENAMES})
      set(TEST_INCLUDE "#include \"${TEST_SOURCE}\"")
      set(TEST_INCLUDES "${TEST_INCLUDES}\n${TEST_INCLUDE}")
  endforeach()

  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/vunit-test-runner.c.in"
    "${SOURCE_FILENAME}"
  )
endfunction()

# create test runner with all tests for c & cxx
set(TEST_SOURCES "test-color.c" "test-parse.c" "test-recv.c")
set(VUINT_TEST_RUNNER_C "${CMAKE_CURRENT_BINARY_DIR}/vunit-test-runner.c")
set(VUINT_TEST_RUNNER_CXX "${CMAKE_CURRENT_BINARY_DIR}/vunit-test-runner.cc")
configure_test_runner("${TEST_SOURCES}" "${VUINT_TEST_RUNNER_C}")
configure_test_runner("${TEST_SOURCES}" "${VUINT_TEST_RUNNER_CXX}")

# re-run cmake configure if test files change
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${TEST_SOURCES})

# create a test runner with parsing support disabled
set(TEST_SOURCES "test-color.c" "test-recv.c")
set(VUINT_TEST_RUNNER_NO_PARSE_C "${CMAKE_CURRENT_BINARY_DIR}/vunit-test-runner-no-parse.c")
configure_test_runner("${TEST_SOURCES}" "${VUINT_TEST_RUNNER_NO_PARSE_C}")

# vunit
add_library(vunit vunit.c)
target_link_libraries(vunit PUBLIC $<$<PLATFORM_ID:Linux>:m>)
target_compile_options(vunit PUBLIC
  $<$<C_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

# vibrant header
add_library(vibrant INTERFACE)
target_include_directories(vibrant INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

# vibrant needs to be compilable in C99, C++11 and C++20+ over many compilers
# and compile time switches. we get very good coverage by compiling all of
# these test runners.
add_test_exe(vtest "${VUINT_TEST_RUNNER_C}" OFF)
add_test_exe(vtest_double_precision "${VUINT_TEST_RUNNER_C}" "VIBRANT_DOUBLE_PRECISION")
add_test_exe(vtest_cc "${VUINT_TEST_RUNNER_CXX}" OFF)
add_test_exe(vtest_cc_double_precision "${VUINT_TEST_RUNNER_CXX}" "VIBRANT_DOUBLE_PRECISION")
add_test_exe(vtest_no_parse "${VUINT_TEST_RUNNER_NO_PARSE_C}" "VIBRANT_NO_PARSE")

# run them all
include(CTest)
enable_testing()
add_test(NAME vtest COMMAND vtest)
add_test(NAME vtest_double_precision COMMAND vtest_double_precision)
add_test(NAME vtest_cc COMMAND vtest_cc)
add_test(NAME vtest_cc_double_precision COMMAND vtest_cc_double_precision)
add_test(NAME vtest_no_parse COMMAND vtest_no_parse)
