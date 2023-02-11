#pragma once

#include <math.h>

#define MAX_DIS 99999  // 距离最大比较值

// 派出的舰队类型 [舰队、打击]
typedef enum {
  ARMY = 0, DARK
} Army_Type;

typedef struct {
  Position pos;        // 舰队当前的位置
  Position vec;        // 舰队的速度向量
  Civi *civi;          // 舰队归属的文明
  Star *target;        // 舰队的目标Star
  float AC;            // 舰队的军事力量
  int army_type;       // 派出类型 [舰队、打击]
  uint8_t arrive;      // 是否到达标志
} Army;

// 计算速度
static void cul_vec(Position *now, Position *to, Position *vector, float vec) {
  // 先获取方向向量长度
  Position p = {to->x - now->x, to->y - now->y};
  float s = sqrtf((p.x * p.x) + (p.y * p.y));
  // 再算速度向量
  vector->x = p.x / s * vec;
  vector->y = p.y / s * vec;
}

// 创建并返回一支舰队
Army *create_army(Civi *civi, Star *f_star, Star *t_star, uint64_t sp, Army_Type type) {
  Army *army = MALLOC(Army, sizeof(Army));
  army->civi = civi;
  army->pos = f_star->pos;
  army->target = t_star;
  // 计算该文明当前技术时的舰队速度
  float v = Compare(civi->Technol, sp) ? 10 : log10(civi->Technol);
  cul_vec(&f_star->pos, &army->target->pos, &army->vec, v);
  army->AC = civi->AC;                     // 将当时文明的AC赋给舰队
  army->army_type = type;
  army->arrive = 0;
  return army;
}

// 获取出发Star
Star *get_target_star(Civi *civi, Array *star_list) {
  Star *temp, *target;                    // 循环中的临时变量
  float dis, tem_dis = MAX_DIS;     // 临时存放最小距离
  // 获取非本文明占领的Star里，离文明中心Star最近的Star
  for (int i = 0; i < star_list->size; ++i) {
    temp = star_list->items[i];
    if (isInArray(civi->domains, temp) < 0) {
      dis = distance_mat(&civi->capital->pos, &temp->pos);
      if (dis < tem_dis) {
        tem_dis = dis;
        target = temp;
      }
    }
  }
  return target;
}

// 获取到达Star
Star *get_launch_star(Star *star, Array *civi_domains_list) {
  Star *temp, *launch;                    // 循环中的临时变量
  float dis, tem_dis = MAX_DIS;     // 临时存放最小距离
  // 获取距离这个Star最近的本文明占据的Star，用作发射位置
  for (int i = 0; i < civi_domains_list->size; ++i) {
    temp = civi_domains_list->items[i];
    dis = distance_mat(&star->pos, &temp->pos);
    if (dis < tem_dis) {
      tem_dis = dis;
      launch = temp;
    }
  }
  return launch;
}

// 舰队航行
void army_refresh(Array *st_list, Army *army) {
  // 如果目的地不存在，视作到达目的地
  if (isInArray(st_list, army->target) < 0) {
    army->arrive = 1;
    return;
  }
  float dis = distance_mat(&army->pos, &army->target->pos);
  // 当一次移动的距离超过与目的地之间的距离时，视作到达目的地
  if (ABS(army->vec.x) > dis || ABS(army->vec.y) > dis) {
    army->arrive = 1;
    return;
  }
  // 更新舰队位置
  army->pos.x += army->vec.x;
  army->pos.y += army->vec.y;
}