#pragma once

#include <stdint.h>
#include "random.c"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) > 0 ? (a) : -(a))
#define Compare(a, b) ((a) >= (b) ? 1 : 0)

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Color;

typedef struct {
  float x;
  float y;
} Position;

// Color初始化
void create_color(Color *c) {
  c->r = rand_i(20, 240);
  c->g = rand_i(20, 240);
  c->b = rand_i(20, 240);
}

// 曼哈顿距离
float distance_mat(Position *a, Position *b) {
  return ABS(a->x - b->x) + ABS(a->y - b->y);
}