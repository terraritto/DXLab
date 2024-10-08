#ifndef __CONSTANT__
#define __CONSTANT__

// 数学定数
static const float PI = 3.1415926535f;
static const float PI2 = 2.0f * PI;
static const float INVPI = 1 / PI;
static const float INVPI2 = 1 / PI2;
static const float EPSILON = 0.001f;

// 制約
#define MAX_RECURSIVE 10 // 最大のRayのDepth
#define MAX_LIGHT_NUM 50 // Lightの最大数
#define MAX_AREA_NUM 100 // AreaLightの最大数

// Light関係
#define MIN_DIST 0.0001f  // 最低保証
#endif