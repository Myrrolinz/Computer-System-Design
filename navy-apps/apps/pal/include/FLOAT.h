#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "assert.h"
#include "common.h"
typedef int FLOAT;

static inline int F2int(FLOAT a) {//浮点数转整数
  if ((a & 0x80000000) == 0) {//正数
    return a >> 16;//右移16位
  }
  else {
    return -((-a) >> 16);//负数
  }
}

static inline FLOAT int2F(int a) {
  if ((a & 0x80000000) == 0) {
    return a << 16;
  }
  else {
    return -((-a) << 16);
  }
}

FLOAT f2F(float);
FLOAT F_mul_F(FLOAT, FLOAT);
FLOAT F_div_F(FLOAT, FLOAT);

static inline FLOAT F_mul_int(FLOAT a, int b) { 
  return F_mul_F(a, int2F(b));
}

static inline FLOAT F_div_int(FLOAT a, int b) {
   return F_div_F(a, int2F(b));
}

FLOAT Fabs(FLOAT);
FLOAT Fsqrt(FLOAT);
FLOAT Fpow(FLOAT, FLOAT);

#endif
