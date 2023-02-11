#pragma once

#include "star.c"

#define Living_up (civi->Living * civi->Living_l * \
  (1 - civi->Living / MIN(civi->All_Res * civi->Res_lr, civi->Technol * civi->Technol_lt)))

typedef struct {
  Star *capital;          // 起源的星系
  Array *domains;         // 占领星系的列表
  double All_Res;         // 总资源量
  double Technol;         // 技术值
  double Living;          // 生命值
  uint32_t Living_Time;   // 已存在时间
  float AC;               // 军力
  float Tech_Exp_Prob;    // 技术爆炸的可能性,按照人类文明平均3000年一次
  float Tech_Exp_t;       // 技术爆炸的年化增长率
  float Dis_Prob;         // 天灾的可能性
  float Dis_k_Tech;       // 天灾时技术退化比
  float Dis_k_Living;     // 天灾时人口衰减比
  float Res_rt;           // 消耗资源的速度,来自技术
  float Res_rl;           // 消耗资源的速度,来自生命
  float Living_l;         // 生命值增加率
  float Res_lr;           // 单位资源可以承载的人口上限
  float Technol_lt;       // 单位技术可以承载的人口上限
  float p_expand;         // 扩张欲望
  float alpha;            // 生命值重要程度
  float beta;             // 技术值重要程度
  float gama;             // 资源值重要程度
  float al;               // 一次派出军队的人口占文明人口的比例
  float at;               // 一次派出军队的技术占文明技术的比例
  Color color;            // 区分颜色
  uint8_t Tech_Exp;       // 是否正在技术爆炸
  uint16_t code;          // 编号
  uint8_t Tech_Rest_time; // 科技革命剩下的时间
  uint8_t is_Dead;        // 是否死亡
} Civi;

// 生成文明
Civi *create_civi(Star *star, uint16_t code) {
  Civi *civi = MALLOC(Civi, sizeof(Civi));
  civi->code = code;
  civi->capital = star;
  create_color(&civi->color);
  civi->Living = 1;
  civi->Technol = 1;
  civi->All_Res = star->resources;
  civi->Living_Time = 0;
  civi->Tech_Exp = 0;
  civi->Tech_Exp_Prob = 0.000334f;
  civi->Tech_Rest_time = 0;
  civi->Tech_Exp_t = 1.02f;
  civi->Dis_Prob = 0.01f;
  civi->Dis_k_Tech = 0.97f;
  civi->Dis_k_Living = 0.95f;
  civi->Res_rt = 0.01f;
  civi->Res_rl = 0.01f;
  civi->Living_l = 0.02f;
  civi->Res_lr = 0.1f;
  civi->Technol_lt = 0.1f;
  civi->p_expand = random() * 0.001f;
  civi->alpha = random();
  civi->beta = random();
  civi->gama = random();
  civi->al = 0.001f;
  civi->at = 0.001f;
  civi->AC = 0;
  civi->is_Dead = 0;
  civi->domains = initArray(1, star);
  star->occupied = civi;
  return civi;
}

// 每回合都需要刷新
void civi_refresh(Civi *civi) {
  // 生命值过低、资源耗尽、技术退化 -> 文明死亡
  if (civi->Living < 0.1 || civi->All_Res < 1 || civi->Technol < 0.1) {
    civi->is_Dead = 1;
    return;
  }
  ++civi->Living_Time;                    // 生存时间增加
  // 如果正在进行技术革命
  if (civi->Tech_Exp) {
    if (civi->Tech_Rest_time > 0) {     // 如果革命还有时间
      --civi->Tech_Rest_time;               // 革命结束年限-1
      civi->Technol *= civi->Tech_Exp_t;    // 革命期间技术增长
    } else civi->Tech_Exp = 0;          // 否则结束革命
  } else if (civi->Tech_Exp_Prob > random()) {
    civi->Tech_Exp = 1;
    civi->Tech_Rest_time = rand_i(10, 100);
  }
  // 发生天灾
  if (civi->Dis_Prob > random()) { // todo:这里可以随机灾害程度
    civi->Living *= civi->Dis_k_Living;
    civi->Technol *= civi->Dis_k_Tech;
  }
  // 文明的生命值增加
  civi->Living += Living_up;
  // 每回合，资源都会消耗
  double turn_use_res = civi->Technol * civi->Res_rt + civi->Living * civi->Res_rl;
  double per_use = turn_use_res / civi->domains->size; // 平均每个Star资源消耗量
  // 更新文明总资源占有
  civi->All_Res = 0;
  Star *d_s;
  for (int i = 0; i < civi->domains->size; ++i) {
    d_s = civi->domains->items[i];
    d_s->resources -= per_use;                         // 减去资源消耗
    civi->All_Res += d_s->resources;                   // 汇总剩余资源
  }
  // 更新军力值
  civi->AC = (float) (civi->al * civi->Living + civi->at * civi->Technol);
}