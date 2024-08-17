#ifndef __CONSTANT__
#define __CONSTANT__

// ���w�萔
static const float PI = 3.1415926535f;
static const float PI2 = 2.0f * PI;
static const float INVPI = 1 / PI;
static const float INVPI2 = 1 / PI2;
static const float EPSILON = 0.001f;

// ����
#define MAX_RECURSIVE 10 // �ő��Ray��Depth
#define MAX_LIGHT_NUM 50 // Light�̍ő吔
#define MAX_AREA_NUM 100 // AreaLight�̍ő吔

// Light�֌W
#define MIN_DIST 0.0001f  // �Œ�ۏ�
#endif