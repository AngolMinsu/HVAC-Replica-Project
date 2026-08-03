#ifndef TEST_ASSERT_H
#define TEST_ASSERT_H

#include <stdio.h>

#ifndef TEST_ASSERT_RETURN
#define TEST_ASSERT_RETURN 1
#endif

#define ASSERT(expr) \
  do { \
    if (!(expr)) { \
      printf("[FAIL] %s:%d ASSERT(%s)\n", __FILE__, __LINE__, #expr); \
      return TEST_ASSERT_RETURN; \
    } \
  } while (0)

#define ASSERT_EQUALS(index, actual, expected) \
  do { \
    long test_actual_value = (long)(actual); \
    long test_expected_value = (long)(expected); \
    if (test_actual_value != test_expected_value) { \
      printf("[FAIL] %s:%d value[%d] actual=%ld expected=%ld\n", \
             __FILE__, __LINE__, (index), test_actual_value, test_expected_value); \
      return 0; \
    } \
  } while (0)

#endif
