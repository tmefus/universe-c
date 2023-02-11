#pragma once

#include <stdint.h>
#include "array.c"
#include "star.c"
#include "civi.c"
#include "army.c"

// 恒星数量 按照当前宽高，最大合适数量为500
static const uint16_t StarCount = 100;

typedef struct {
  Array *Star_List;         // 保存有宇宙内全部恒星的列表
  Array *Civi_List;         // 保存有宇宙内全部文明的列表
  Array *Army_List;         // 保存有宇宙内全部的军队情况
  uint64_t TC_super;        // 技术值超过这个，达到宇宙最大速度
  float Gen_Civi_Prob;      // 每个无主的恒星系诞生生命的概率
  float broadcast_dis;      // 多少距离内的可以听到宇宙广播
  uint32_t Cvi_Number;      // 迄今为止有多少文明出现
  uint32_t Now_Years;       // 记录迄今为止的时间
  uint16_t TC_Sail;         // 星际时代的技术值门槛
} Universe;

// 创建一个Universe并返回
Universe *create_universe(uint16_t wid, uint16_t hei) {
  Universe *uvs = MALLOC(Universe, sizeof(Universe));
  uvs->Star_List = gen_stars(wid, hei, StarCount);
  uvs->Civi_List = initArray(7, NULL);
  uvs->Army_List = initArray(7, NULL);
  uvs->TC_super = 10000000000;
  uvs->TC_Sail = 1000;
  uvs->Gen_Civi_Prob = 0.001f;
  uvs->Cvi_Number = 0;
  uvs->Now_Years = 0;
  uvs->broadcast_dis = 400;
  return uvs;
}

// 刷新恒星列表以产生文明
static void refresh_star_list(Universe *uvs) {
  Star *star;
  for (int i = 0; i < uvs->Star_List->size; ++i) {
    star = uvs->Star_List->items[i];
    if (star->occupied == NULL && uvs->Gen_Civi_Prob > random()) {
      Civi *c = create_civi(star, ++uvs->Cvi_Number);
      addArray(uvs->Civi_List, c);
    }
  }
}

// 刷新文明列表以发展文明
static void refresh_civi_list(Array *list) {
  for (int i = 0; i < list->size; ++i) {
    civi_refresh(list->items[i]);
  }
}

// 刷新军队
static void refresh_army_list(Universe *uvs) {
  Civi *civi;
  // 是否有新舰队发出发
  for (int i = 0; i < uvs->Civi_List->size; ++i) {
    civi = uvs->Civi_List->items[i];
    // 技术大于星际航行标准,意欲扩张,且未占据全部Star
    if (Compare(civi->Technol, uvs->TC_Sail) && civi->p_expand > random()
        && civi->domains->size < uvs->Star_List->size) {
      // 获取到达Star
      Star *t_star = get_target_star(civi, uvs->Star_List);
      // 获取出发Star
      Star *f_star = get_launch_star(t_star, civi->domains);
      // 创建并放入舰队列表
      Army *army = create_army(civi, f_star, t_star, uvs->TC_super, ARMY);
      addArray(uvs->Army_List, army);  // 把舰队加入宇宙的舰队列表
    }
  }
  // 刷新已有的舰队
  for (int i = 0; i < uvs->Army_List->size; ++i) {
    army_refresh(uvs->Star_List, uvs->Army_List->items[i]);
  }
}

// 刷新舰队事件
static void refresh_army_event(Universe *uvs) {
  Army *army;
  for (int i = 0; i < uvs->Army_List->size; ++i) {
    army = uvs->Army_List->items[i];
    if (army->arrive) {                   // 当军队抵达目的地
      // 目的地无了(目的地被之上的文明耗光了资源)
      if (isInArray(uvs->Star_List, army->target) < 0) continue;
      // 主文明没了(派出舰队的文明本体死亡了)
      if (isInArray(uvs->Civi_List, army->civi) < 0) {
        if (army->target->occupied == NULL) { // 目的地是无主之地就占据并延续文明
          Civi *c = create_civi(army->target, ++uvs->Cvi_Number);
          addArray(uvs->Civi_List, c);
        }
        continue;
      }
      // 打击无主之地(要打击的文明在打击到达前消亡了)
      if (army->target->occupied == NULL && army->army_type == DARK) continue;
      // 占领无主之地
      if (army->target->occupied == NULL && army->army_type == ARMY) {
        army->target->occupied = army->civi;
        addArray(army->civi->domains, army->target);
        continue;
      }
      // 打击这个Star上的文明
      if (army->target->occupied != NULL && army->army_type == DARK) {
        Civi *this_civi = army->target->occupied;
        if (this_civi->domains->size == 1) this_civi->is_Dead = 1; // 如果该文明只有这个星系则死亡
        else removeByObj(this_civi->domains, army->target);
        army->target->occupied = NULL;                             // 最后将该Star设为无主之地
        continue;
      }
      // 占领这儿的是自己(两个舰队先后到达,后到达的舰队会遇到这种情况, 防止自己打击自己)
      if (army->target->occupied == army->civi) continue;
      // 占领这儿的是别人
      if (army->target->occupied != NULL && army->army_type == ARMY) {
        Civi *enemy = army->target->occupied;
        // 开始进行透明博弈
        if (random() > 0.5) {   // 选择接触的概率
          float winAC = army->AC, loseAC = enemy->AC;  // 双方军力
          double loseRC = enemy->All_Res;
          Civi *winCivi = army->civi, *loseCivi = enemy;
          // 判断出胜败双方和胜败方的资源
          if (army->AC < enemy->AC) {
            winAC = enemy->AC;
            loseAC = army->AC;
            loseRC = army->civi->All_Res;
            winCivi = enemy;
            loseCivi = army->civi;
          }
          // 计算出输赢可能性大小
          double win_p = 1 - 0.5 / (1 + (loseCivi->gama * (double) loseRC) / (loseCivi->alpha * winAC));
          double lose_p = 1 - 0.5 / (1 + (winCivi->gama * (double) loseRC) / (winCivi->alpha * loseAC));
          // 双方都没选择战争, 什么都不发生
          if (win_p > random() && lose_p > random()) continue;
          // 否则发生战争
          loseCivi->is_Dead = 1;                                // 败方退场
          Star *s;
          for (int j = 0; j < loseCivi->domains->size; ++j) {   // 转移败方占领的Star
            s = loseCivi->domains->items[j];
            s->occupied = winCivi;
            addArray(winCivi->domains, s);
          }
          // 胜者会受伤
          winCivi->Living -= loseAC;
        } else {
          // 选择打击
          Civi *c;
          for (int j = 0; j < uvs->Civi_List->size; ++j) {
            c = uvs->Civi_List->items[j];
            if (c != army->civi && Compare(c->Technol, uvs->TC_super)
                && 0.090909 > random() // 技术爆炸反败为胜的概率
                && distance_mat(&army->target->pos, &c->capital->pos) < uvs->broadcast_dis) {
              Army *dark_army = create_army(c, c->capital, army->target, uvs->TC_super, DARK);
              addArray(uvs->Army_List, dark_army);
            }
          }
        }
      }
    }
  }
}

// 清除已到达的舰队
static void remove_arrive_army(Universe *uvs) {
  Array *temp = initArray(7, NULL);
  Army *army;
  for (int i = 0; i < uvs->Army_List->size; ++i) {
    army = uvs->Army_List->items[i];
    if (!army->arrive) addArray(temp, army);
  }
  deleteArray(uvs->Army_List);
  uvs->Army_List = temp;
}

// 清除灭亡的文明
static void remove_dead_civi(Universe *uvs) {
  Array *temp = initArray(7, NULL);
  Civi *civi;
  Star *star;
  for (int i = 0; i < uvs->Civi_List->size; ++i) {
    civi = uvs->Civi_List->items[i];
    if (civi->is_Dead) {
      for (int j = 0; j < civi->domains->size; ++j) {
        star = civi->domains->items[j];
        star->occupied = NULL;
      }
    } else {
      addArray(temp, civi);
    }
  }
  deleteArray(uvs->Civi_List);
  uvs->Civi_List = temp;
}

// 清除资源枯竭的Star
static void remove_empty_star(Universe *uvs) {
  Array *temp = initArray(7, NULL);
  Star *star;
  for (int i = 0; i < uvs->Star_List->size; ++i) {
    star = uvs->Star_List->items[i];
    if (star->resources > 1) addArray(temp, star);
  }
  deleteArray(uvs->Star_List);
  uvs->Star_List = temp;
}

// 宇宙刷新
void uvs_refresh(Universe *uvs) {
  ++uvs->Now_Years;
  // 随机生成文明
  refresh_star_list(uvs);
  // 所有文明发展
  refresh_civi_list(uvs->Civi_List);
  // 派出舰队
  refresh_army_list(uvs);
  // 刷新舰队事件
  refresh_army_event(uvs);
  // 清除已到达的舰队
  remove_arrive_army(uvs);
  // 清除灭亡的文明
  remove_dead_civi(uvs);
  // 清除资源枯竭的Star
  remove_empty_star(uvs);
}