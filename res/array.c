#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 平台指针的大小
static const int P_S = sizeof(void *);

// 打印错误信息并退出
#define ERROR_EXIT(...) {              \
  printf(" __%s__ : ", __FUNCTION__);  \
  printf(__VA_ARGS__); exit(-1);       \
}

// 申请or扩展内存
#define MALLOC(type, size) (type*)reallocate(NULL, size)
#define RELLOC(type, ptr, size) (type*)reallocate(ptr, size)

typedef struct {
  uint16_t capacity;     // 动态数组容量
  uint16_t size;         // 动态数组实际大小
  void **items;          // 维护在堆区创建的指针数组
} Array;

// 内存扩展
static void *reallocate(void *ptr, size_t size) {
  void *re = (ptr == NULL) ? malloc(size) : realloc(ptr, size);
  if (re == NULL) ERROR_EXIT("内存分配错误")
  return re;
}

static void ensure_capacity(Array *arr) {
  if (arr->size == arr->capacity) {       // array数组已满，需要申请更大空间
    int newCapacity = arr->capacity * 2;  // 动态数组新容量
    arr->items = RELLOC(void *, arr->items, P_S * newCapacity);
    arr->capacity = newCapacity;
  }
}

void addArray(Array *arr, void *data) {
  if (arr == NULL || data == NULL) ERROR_EXIT("空指针错误")
  ensure_capacity(arr); // 确保数组容量足够
  arr->items[arr->size++] = data;
}

Array *initArray(uint16_t capacity, void *init_item) {
  if (capacity < 0 || capacity > UINT16_MAX) ERROR_EXIT("初始化大小错误")
  capacity = capacity < 2 ? 2 : capacity;        // 最小容量为1
  Array *arr = MALLOC(Array, sizeof(Array));
  arr->capacity = capacity;                      // 将动态数组容量初始化为函数入参
  arr->size = 0;                                 // 将动态数组实际大小初始化为0
  arr->items = MALLOC(void *, P_S * capacity);   // 在堆区申请二级指针p的内存
  if (init_item != NULL) arr->items[arr->size++] = init_item; // 放入初始化元素
  return arr;
}

void insertArray(Array *arr, int index, void *data) {
  if (arr == NULL || data == NULL) ERROR_EXIT("空指针错误")
  if (index < 0 || index >= arr->size) ERROR_EXIT("索引错误")
  ensure_capacity(arr); // 确保数组容量足够
  // 因为 malloc realloc申请的内存是连续的，所以可以直接操作连续的内存空间
  memmove(arr->items + index + 1, arr->items + index, P_S * (arr->size - index));
  arr->items[index] = data;
  arr->size++;
}

int isInArray(Array *arr, void *obj) {
  if (arr == NULL || obj == NULL) ERROR_EXIT("空指针错误")
  for (int i = 0; i < arr->size; ++i) {
    if (arr->items[i] == obj) return i;  // 因为items储存的是地址指针，所以比较指针就可以
  }
  return -1;
}

void removeByIdx(Array *arr, int index) {
  if (arr == NULL) ERROR_EXIT("空指针错误")
  if (arr->size == 0) ERROR_EXIT("数组为空")
  if (abs(index) >= arr->size || index < 0) ERROR_EXIT("下标错误")
  free(arr->items[index]);   // 删除元素
  // 因为 malloc realloc申请的内存是连续的，所以可以直接操作连续的内存空间
  memmove(arr->items + index, arr->items + index + 1, P_S * (arr->size - index));
  arr->size--;                      // 原实际数组大小减1
  int hc = arr->capacity / 2;       // 调整动态数组空间
  if (arr->size == hc && hc != 0) { // 元素数量为容量一半,新容量不能为0
    arr->capacity = hc;
    arr->items = RELLOC(void *, arr->items, P_S * hc);
  }
}

// 只删除数组中第一个相同的obj
void removeByObj(Array *arr, void *obj) {
  int idx = isInArray(arr, obj);
  if (idx != -1) removeByIdx(arr, idx);
}

// 释放array
void deleteArray(Array *arr) {      // 释放占用的内存即可
  if (arr) {
    if (arr->items) {
      free(arr->items);
      arr->items = NULL;
    }
    free(arr);
    arr = NULL;
  }
}