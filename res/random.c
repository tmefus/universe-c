#pragma once

#include <stdlib.h>
#include <time.h>

#define Init_Rand() srand((unsigned) time(NULL))

// 小整数随机数
static int rand_i(int low, int high) {
  if (low > high) {
    int t;
    t = low;
    low = high;
    high = t;
  }
  double r = (double) rand() / RAND_MAX;
  return low + (int) (r * (high - low));
}

// [0~1] 随机数
static float random() {
  return (float) rand() / RAND_MAX; // 获取[0~1]
}

// 大随机数
static float rand_float(int low, int high) {
  if (low > high) {
    int t;
    t = low;
    low = high;
    high = t;
  }
  double r = (double) rand() / RAND_MAX; // 获取[0~1]
  return (float) (low + r * (high - low));
}

// 大随机数
static double rand_double(double low, double high) {
  double r;
  if (low > high) {
    r = low;
    low = high;
    high = r;
  }
  r = (double) rand() / RAND_MAX; // 获取[0~1]
  return low + r * (high - low);
}