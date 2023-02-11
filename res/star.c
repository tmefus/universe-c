#pragma once

#include "define.c"
#include "random.c"
#include "array.c"

static const int StarSize = 16;  // 恒星数量
static const float Density = 50;   // 恒星最小曼哈顿间距

typedef struct {
  Position pos;             // Star的位置
  void *occupied;           // 占据Star的文明
  double resources;        // Star的资源量
} Star;

// 创建一个Star并返回
Star *create_star(Position *p, double res) {
  Star *s = MALLOC(Star, sizeof(Star));
  s->pos.x = p->x;
  s->pos.y = p->y;
  s->resources = res;
  s->occupied = NULL;
  return s;
}

// 判断一个点是否位置合适
static uint8_t is_good_pos(Position *p, Position ps[], uint32_t size) {
  for (int i = 0; i < size; ++i) {
    float dis = distance_mat(p, &ps[i]);
    if (dis < Density) return 0;
  }
  return 1;
}

// 生成Star
void *gen_stars(uint16_t wid, uint16_t hei, const uint16_t count) {
  Array *list = initArray(count, NULL);
  Position pos, pos_s[count];
  double r;
  for (uint16_t i = 0; i < count;) {
    pos.x = rand_float(StarSize + 8, wid - (StarSize + 8)); // 距离边界一个点大小
    pos.y = rand_float(StarSize + 8, hei - (StarSize + 8));
    if (is_good_pos(&pos, pos_s, i)) {   // 循环比较生成的点和已经有的点位置是否合适
      pos_s[i++] = pos;
      r = rand_double(1e7, 1e15);                    // 随机资源 (1e7, 1e15)
      addArray(list, create_star(&pos, r));
    }
  }
  return list;
}