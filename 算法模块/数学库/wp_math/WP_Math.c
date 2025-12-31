/* Copyright (c)  2019-2025 Wuhan Nameless Innovation Technology Co.,Ltd. All rights reserved.*/
/*----------------------------------------------------------------------------------------------------------------------/
                开源并不等于免费，先驱者的历史已经证明，在国内这个环境下，毫无收益的开源，单靠坊间个人爱好者，自发地参与项
								目完善的方式行不通，好的开源项目需要请专职人员做好售后服务、手把手教学、统计用户反馈需求、在实践中完成对产
								品的一次次完善与迭代升级。经过综合考虑，无名飞控开源代码中，程序仅保留公司正版激活功能代码，版本激活无实际
								功能，属于公司产品常规出厂操作，不影响客户学习，其余代码全部开放给客户学习，客户移植和二次开发代码请保留代
								码版权。
-----------------------------------------------------------------------------------------------------------------------/
*               本程序只供购买者学习使用，版权著作权属于无名科创团队，无名科创团队将飞控程序源码提供给购买者，
*               购买者要为无名科创团队提供保护，未经作者许可，不得将源代码提供给他人，不得将源代码放到网上供他人免费下载， 
*               更不能以此销售牟利，如发现上述行为，无名科创团队将诉之以法律解决！！！
-----------------------------------------------------------------------------------------------------------------------
*                                                 为什么选择无名创新？
*                                         感动人心价格厚道，最靠谱的开源飞控；
*                                         国内业界良心之作，最精致的售后服务；
*                                         追求极致用户体验，高效进阶学习之路；
*                                         萌新不再孤单求索，合理把握开源尺度；
*                                         响应国家扶贫号召，促进教育体制公平；
*                                         新时代奋斗最出彩，建人类命运共同体。 
-----------------------------------------------------------------------------------------------------------------------
*               生命不息、奋斗不止；前人栽树，后人乘凉！！！
*               开源不易，且学且珍惜，祝早日逆袭、进阶成功！！！
*               学习优秀者，简历可推荐到DJI、ZEROTECH、XAG、AEE、GDU、AUTEL、EWATT、HIGH GREAT等公司就业
*               求职简历请发送：15671678205@163.com，需备注求职意向单位、岗位、待遇等
*               无名科创开源飞控QQ群：2号群465082224、1号群540707961（人员已满）
*               CSDN博客：http://blog.csdn.net/u011992534
*               优酷ID：NamelessCotrun无名小哥
*               B站教学视频：https://space.bilibili.com/67803559/#/video
*               客户使用心得、改进意见征集贴：http://www.openedv.com/forum.php?mod=viewthread&tid=234214&extra=page=1
*               淘宝店铺：https://shop348646912.taobao.com/?spm=2013.1.1000126.2.5ce78a88ht1sO2
*               百度贴吧:无名科创开源飞控
*               公司官网:www.nameless.tech
*               无名创新国内首款TI开源飞控设计初衷、知乎专栏:https://zhuanlan.zhihu.com/p/54471146
*               修改日期:2020/03/24                   
*               版本：躺赢者PRO——CarryPilot_V3.0.1
*               版权所有，盗版必究。
*               Copyright(C) 2019-2025 武汉无名创新科技有限公司 
*               All rights reserved
-----------------------------------------------------------------------------------------------------------------------
*               重要提示：
*               正常淘宝咸鱼转手的飞控、赠送朋友、传给学弟的都可以进售后群学习交流，
*               不得直接在网上销售无名创新资料，无名创新代码有声明版权，他人不得将
*               资料代码传网上供他人下载，不得以谋利为目的销售资料代码，发现有此操
*               作者，公司会提前告知，请1天内及时处理，否则你的学校、单位、姓名、电
*               话、地址信息会被贴出在公司官网、官方微信公众平台、官方技术博客、知乎
*               专栏以及淘宝店铺首页予以公示公告，此种所作所为，会成为个人污点，影响
*               升学、找工作、社会声誉、很快就很在无人机界出名，后果很严重。
*               因此行为给公司造成重大损失者，会以法律途径解决，感谢您的合作，谢谢！！！
----------------------------------------------------------------------------------------------------------------------*/
#include "math.h"
#include "WP_Math.h"
// a varient of asin() that checks the input ranges and ensures a
// valid angle as output. If nan is given as input then zero is
// returned.
float safe_asin(float v)
{
  if (isnan(v)) {
    return 0.0;
  }
  if (v >= 1.0f) {
    return PI/2;
  }
  if (v <= -1.0f) {
    return -PI/2;
  }
  return asinf(v);
}


// a varient of sqrt() that checks the input ranges and ensures a
// valid value as output. If a negative number is given then 0 is
// returned. The reasoning is that a negative number for sqrt() in our
// code is usually caused by small numerical rounding errors, so the
// real input should have been zero
float safe_sqrt(float v)
{
  float ret = sqrtf(v);
  if (isnan(ret)) {
    return 0;
  }
  return ret;
}

// a faster varient of atan.  accurate to 6 decimal places for values between -1 ~ 1 but then diverges quickly
float fast_atan(float v)
{
  float v2 = v*v;
  return (v*(1.6867629106f + v2*0.4378497304f)/(1.6867633134f + v2));
}


#define FAST_ATAN2_PIBY2_FLOAT  1.5707963f
// fast_atan2 - faster version of atan2
//      126 us on AVR cpu vs 199 for regular atan2
//      absolute error is < 0.005 radians or 0.28 degrees
//      origin source: https://gist.github.com/volkansalma/2972237/raw/
float fast_atan2(float y, float x)
{
  if (x == 0.0f) {
    if (y > 0.0f) {
      return FAST_ATAN2_PIBY2_FLOAT;
    }
    if (y == 0.0f) {
      return 0.0f;
    }
    return -FAST_ATAN2_PIBY2_FLOAT;
  }
  float atan;
  float z = y/x;
  if (fabs( z ) < 1.0f) {
    atan = z / (1.0f + 0.28f * z * z);
    if (x < 0.0f) {
      if (y < 0.0f) {
        return atan - PI;
      }
      return atan + PI;
    }
  } else {
    atan = FAST_ATAN2_PIBY2_FLOAT - (z / (z * z + 0.28f));
    if (y < 0.0f) {
      return atan - PI;
    }
  }
  return atan;
}


// constrain a value
float constrain_float(float amt, float low, float high) 
{
  // the check for NaN as a float prevents propogation of
  // floating point errors through any function that uses
  // constrain_float(). The normal float semantics already handle -Inf
  // and +Inf
  if (isnan(amt)) {
    return (low+high)*0.5f;
  }
  return ((amt)<(low)?(low):((amt)>(high)?(high):(amt)));
}

// constrain a int16_t value
int16_t constrain_int16(int16_t amt, int16_t low, int16_t high) {
  return ((amt)<(low)?(low):((amt)>(high)?(high):(amt)));
}

// constrain a int32_t value
int32_t constrain_int32(int32_t amt, int32_t low, int32_t high) {
  return ((amt)<(low)?(low):((amt)>(high)?(high):(amt)));
}

// degrees -> radians
float radians(float deg) {
  return deg * DEG_TO_RAD;
}

// radians -> degrees
float degrees(float rad) {
  return rad * RAD_TO_DEG;
}

// square
float sq(float v) {
  return v*v;
}

// 2D vector length
float pythagorous2(float a, float b) {
  return sqrtf(sq(a)+sq(b));
}

// 3D vector length
float pythagorous3(float a, float b, float c) {
  return sqrtf(sq(a)+sq(b)+sq(c));
}



//translate from ADI's dsp library.
//////////////////////////////////////////////////////////////////////////
//Get fraction and integer parts of floating point
float Modf(float x, float *i)
{
  float y;
  float fract;
  
  y = x;
  if (x < (float)0.0){
    y = -y;
  }
  
  if (y >= (float)16777216.0f){
    *i = x;
    return (float)0.0f;
  }
  
  if (y < (float)1.0f){
    *i = (float)0.0f;
    return x;
  }
  
  y = (float)((long)(y));
  
  if (x < (float)0.0f){
    y = -y;
  }
  
  fract = x - y;
  *i = y;
  
  return fract;
}

float FastPow(float x,float y)
{
  float tmp;
  float znum, zden, result;
  float g, r, u1, u2, v, z;
  long m, p, negate, y_int, n;
  long mi, pi, iw1;
  float y1, y2, w1, w2, w;
  float *a1, *a2;
  float xtmp;
  long *lPtr = (long *)&xtmp;
  float *fPtr = &xtmp;
  static const long a1l[] =  {0,            /* 0.0 */
  0x3f800000,   /* 1.0 */
  0x3f75257d,   /* 0.9576032757759 */
  0x3f6ac0c6,   /* 0.9170039892197 */
  0x3f60ccde,   /* 0.8781260251999 */
  0x3f5744fc,   /* 0.8408963680267 */
  0x3f4e248c,   /* 0.8052451610565 */
  0x3f45672a,   /* 0.7711054086685 */
  0x3f3d08a3,   /* 0.7384130358696 */
  0x3f3504f3,   /* 0.7071067690849 */
  0x3f2d583e,   /* 0.6771277189255 */
  0x3f25fed6,   /* 0.6484197378159 */
  0x3f1ef532,   /* 0.6209288835526 */
  0x3f1837f0,   /* 0.5946035385132 */
  0x3f11c3d3,   /* 0.5693942904472 */
  0x3f0b95c1,   /* 0.5452538132668 */
  0x3f05aac3,   /* 0.5221368670464 */
  0x3f000000};  /* 0.5 */
  static const long a2l[] =  {0,            /* 0.0 */
  0x31a92436,   /* 4.922664054163e-9 */
  0x336c2a94,   /* 5.498675648141e-8 */
  0x31a8fc24,   /* 4.918108587049e-9 */
  0x331f580c,   /* 3.710015050729e-8 */
  0x336a42a1,   /* 5.454296925222e-8 */
  0x32c12342,   /* 2.248419050943e-8 */
  0x32e75623,   /* 2.693110978669e-8 */
  0x32cf9890};  /* 2.41673490109e-8 */
  
  a1 = (float *)a1l;
  a2 = (float *)a2l; 
  negate = 0;
  
  if (x == (float)0.0){
    if (y == (float)0.0){
      return (float)1.0;
    }
    else if (y > (float)0.0){
      return (float)0.0;
    }
    else{
      return (float)FLT_MAX;
    }
  }
  else if (x < (float)0.0){
    y_int = (long)(y);
    if ((float)(y_int) != y){
      return (float)0.0;
    }
    
    x = -x;
    negate = y_int & 0x1;
  }
  
  xtmp = x;
  m = (*lPtr >> 23);
  m = m - 126;
  
  *lPtr = (*lPtr & 0x807fffff) | (126 << 23);
  g = *fPtr;
  
  p = 1;
  if (g <= a1[9]){
    p = 9;
  }
  if (g <= a1[p + 4]){
    p = p + 4;
  }
  if (g <= a1[p + 2]){
    p = p + 2;
  }
  
  p = p + 1;
  znum = g - a1[p];
  znum = znum - a2[p >> 1];
  
  zden = g + a1[p];
  
  p = p - 1;
  
  z = znum / zden;
  z = z + z;
  
  v = z * z;
  
  r = POWP_COEF2 * v;
  r = r + POWP_COEF1;
  r = r * v;
  r = r * z;
  
  r = r + LOG2E_MINUS1 * r;
  u2 = LOG2E_MINUS1 * z;
  u2 = r + u2;
  u2 = u2 + z;
  
  u1 = (float)((m * 16) - p);
  u1 = u1 * 0.0625f;
  
  Modf(16.0f * y, &(y1));
  y1 = y1 * 0.0625f;
  
  y2 = y - y1;
  
  w = u1 * y2;
  tmp = u2 * y;
  w = tmp + w;
  
  Modf(16.0f * w, &(w1));
  w1 = w1 * 0.0625f;
  
  w2 = w - w1;
  
  w = u1 * y1;
  w = w + w1;
  
  Modf(16.0f * w, &(w1));
  w1 = w1 * 0.0625f;
  
  tmp = w - w1;
  w2 = w2 + tmp;
  
  Modf(16.0f * w2, &(w));
  w = w * 0.0625f;
  
  tmp = w1 + w;
  tmp = 16.0f * tmp;
  iw1 = (long)(tmp);
  
  w2 = w2 - w;
  
  if (iw1 > POW_BIGNUM){
    result = (float)FLT_MAX;
    if (negate == 1){
      result = -result;
    }
    return result;
  }
  
  if (w2 > 0){
    w2 = w2 - 0.0625f;
    iw1++;
  }
  
  if (iw1 < POW_SMALLNUM){
    return (float)0.0;
  }
  
  if (iw1 < 0){
    mi = 0;
  }
  else{
    mi = 1;
  }
  n = iw1 / 16;
  mi = mi + n;
  pi = (mi * 16) - iw1;
  
  z = POWQ_COEF5 * w2;
  z = z + POWQ_COEF4;
  z = z * w2;
  z = z + POWQ_COEF3;
  z = z * w2;
  z = z + POWQ_COEF2;
  z = z * w2;
  z = z + POWQ_COEF1;
  z = z * w2;
  
  z = z * a1[pi + 1];
  z = a1[pi + 1] + z;
  
  fPtr = &z;
  lPtr = (long *)fPtr;
  n = (*lPtr >> 23) & 0xff;
  n = n - 127;
  mi = mi + n;
  mi = mi + 127;
  
  mi = mi & 0xff;
  *lPtr = *lPtr & (0x807fffff);
  *lPtr = *lPtr | mi << 23;
  
  result = *fPtr;
  
  if (negate){
    result = -result;
  }
  
  return result;
}
//
float FastTan(float x)
{
  long n;
  float xn;
  float f, g;
  float x_int, x_fract;
  float result;
  float xnum, xden;
  
  if ((x > (float)X_MAX) || (x < (float)-X_MAX)){
    return (float)0.0;
  }
  
  x_int = (float)((long)(x));
  x_fract = x - x_int;
  
  g = (float)0.5;
  if (x <= (float)0.0){
    g = -g;
  }
  n = (long)(x * (float)INV_PI_2 + g);
  xn = (float)(n);
  
  f = x_int - xn * PI_2_C1;
  f = f + x_fract;
  f = f - xn * PI_2_C2;
  f = f - xn * PI_2_C3;
  
  if (f < (float)0.0){
    g = -f;
  }
  else{
    g = f;
  }
  if (g < (float)EPS_FLOAT){
    if (n & 0x0001){
      result = -1.0f / f;
    }
    else{
      result = f;
    }            
    return result;
  }
  
  g = f * f;
  xnum = g * TANP_COEF2;
  xnum = xnum + TANP_COEF1;
  xnum = xnum * g;
  xnum = xnum * f;
  xnum = xnum + f;
  
  xden = g * TANQ_COEF2;
  xden = xden + TANQ_COEF1;
  xden = xden * g;
  xden = xden + TANQ_COEF0;
  
  if (n & 0x0001){
    result = xnum;
    xnum = -xden;
    xden = result;
  }
  result = xnum / xden;
  return result;
}
//
float FastLn(float x)
{
  union { unsigned int i; float f;} e;
  float xn;
  float	z;
  float	w;
  float	a;
  float	b;
  float	r;
  float	result;
  float znum, zden;
  
  int exponent = (*((int*)&x) & 0x7F800000) >> 23;
  e.i = (*((int*)&x) & 0x3F800000);
  
  if(e.f > ROOT_HALF){
    znum = e.f - 1.0f;
    zden = e.f * 0.5f + 0.5f;
  }
  else{
    exponent -= 1;
    znum = e.f - 0.5f;
    zden = e.f * 0.5f + 0.5f;
  }
  xn = (float)exponent;
  z = znum / zden;
  w = z * z;
  a = (LOGDA_COEF2 * w + LOGDA_COEF1) * w + LOGDA_COEF0;
  b = ((w + LOGDB_COEF2) * w + LOGDB_COEF1) * w + LOGDB_COEF0;
  r = a / b * w * z + z;
  result = xn * LN2_DC1 + r;
  r = xn * LN2_DC2;
  result += r;
  r = xn * LN2_DC3;
  result += r;
  return result;
}

float FastAsin(float x)
{
  float y, g;
  float num, den, result;
  long i;
  float sign = 1.0;
  
  y = x;
  if (y < (float)0.0){
    y = -y;
    sign = -sign;
  }
  
  if (y > (float)0.5){
    i = 1;
    if (y > (float)1.0){
      result = 0.0;
      return result;
    }    
    g = (1.0f - y) * 0.5f;
    y = -2.0f * FastSqrt(g);
  }
  else{
    i = 0;
    if (y < (float)EPS_FLOAT){
      result = y;
      if (sign < (float)0.0){
        result = -result;
      }
      return result;
    }
    g = y * y;
  }
  num = ((ASINP_COEF3 * g + ASINP_COEF2) * g + ASINP_COEF1) * g;
  den = ((g + ASINQ_COEF2) * g + ASINQ_COEF1) * g + ASINQ_COEF0;
  result = num / den;
  result = result * y + y;
  if (i == 1){
    result = result + (float)PI_2;
  }
  if (sign < (float)0.0){
    result = -result;
  }
  return result;
}

float FastAtan2(float y, float x)
{
  float f, g;
  float num, den;
  float result;
  int n;
  
  static const float a[4] = {0, (float)PI_6, (float)PI_2, (float)PI_3};
  
  if (x == (float)0.0){
    if (y == (float)0.0){
      result = 0.0;
      return result;
    }
    
    result = (float)PI_2;
    if (y > (float)0.0){
      return result;
    }
    if (y < (float)0.0){
      result = -result;
      return result;
    }
  }
  n = 0;
  num = y;
  den = x;
  
  if (num < (float)0.0){
    num = -num;
  }
  if (den < (float)0.0){
    den = -den;
  }
  if (num > den){
    f = den;
    den = num;
    num = f;
    n = 2;
  }
  f = num / den;
  
  if (f > (float)TWO_MINUS_ROOT3){
    num = f * (float)SQRT3_MINUS_1 - 1.0f + f;
    den = (float)SQRT3 + f;
    f = num / den;
    n = n + 1;
  }
  
  g = f;
  if (g < (float)0.0){
    g = -g;
  }
  
  if (g < (float)EPS_FLOAT){
    result = f;
  }
  else{
    g = f * f;
    num = (ATANP_COEF1 * g + ATANP_COEF0) * g;
    den = (g + ATANQ_COEF1) * g + ATANQ_COEF0;
    result = num / den;
    result = result * f + f;
  }
  if (n > 1){
    result = -result;
  }
  result = result + a[n];
  
  if (x < (float)0.0){
    result = PI - result;
  }
  if (y < (float)0.0){
    result = -result;
  }
  return result;
}

// Quake inverse square root
float FastSqrtI(float x)
{
  //////////////////////////////////////////////////////////////////////////
  //less accuracy, more faster
  /*
  L2F l2f;
  float xhalf = 0.5f * x;
  l2f.f = x;
  
  l2f.i = 0x5f3759df - (l2f.i >> 1);
  x = l2f.f * (1.5f - xhalf * l2f.f * l2f.f);
  return x;
  */
  //////////////////////////////////////////////////////////////////////////
  union { unsigned int i; float f;} l2f;
  l2f.f = x;
  l2f.i = 0x5F1F1412 - (l2f.i >> 1);
  return l2f.f * (1.69000231f - 0.714158168f * x * l2f.f * l2f.f);
}

float FastSqrt(float x)
{
  return x * FastSqrtI(x);
}

#define FAST_SIN_TABLE_SIZE 512

const float sinTable[FAST_SIN_TABLE_SIZE + 1] = {
  0.00000000f, 0.01227154f, 0.02454123f, 0.03680722f, 0.04906767f, 0.06132074f,
  0.07356456f, 0.08579731f, 0.09801714f, 0.11022221f, 0.12241068f, 0.13458071f,
  0.14673047f, 0.15885814f, 0.17096189f, 0.18303989f, 0.19509032f, 0.20711138f,
  0.21910124f, 0.23105811f, 0.24298018f, 0.25486566f, 0.26671276f, 0.27851969f,
  0.29028468f, 0.30200595f, 0.31368174f, 0.32531029f, 0.33688985f, 0.34841868f,
  0.35989504f, 0.37131719f, 0.38268343f, 0.39399204f, 0.40524131f, 0.41642956f,
  0.42755509f, 0.43861624f, 0.44961133f, 0.46053871f, 0.47139674f, 0.48218377f,
  0.49289819f, 0.50353838f, 0.51410274f, 0.52458968f, 0.53499762f, 0.54532499f,
  0.55557023f, 0.56573181f, 0.57580819f, 0.58579786f, 0.59569930f, 0.60551104f,
  0.61523159f, 0.62485949f, 0.63439328f, 0.64383154f, 0.65317284f, 0.66241578f,
  0.67155895f, 0.68060100f, 0.68954054f, 0.69837625f, 0.70710678f, 0.71573083f,
  0.72424708f, 0.73265427f, 0.74095113f, 0.74913639f, 0.75720885f, 0.76516727f,
  0.77301045f, 0.78073723f, 0.78834643f, 0.79583690f, 0.80320753f, 0.81045720f,
  0.81758481f, 0.82458930f, 0.83146961f, 0.83822471f, 0.84485357f, 0.85135519f,
  0.85772861f, 0.86397286f, 0.87008699f, 0.87607009f, 0.88192126f, 0.88763962f,
  0.89322430f, 0.89867447f, 0.90398929f, 0.90916798f, 0.91420976f, 0.91911385f,
  0.92387953f, 0.92850608f, 0.93299280f, 0.93733901f, 0.94154407f, 0.94560733f,
  0.94952818f, 0.95330604f, 0.95694034f, 0.96043052f, 0.96377607f, 0.96697647f,
  0.97003125f, 0.97293995f, 0.97570213f, 0.97831737f, 0.98078528f, 0.98310549f,
  0.98527764f, 0.98730142f, 0.98917651f, 0.99090264f, 0.99247953f, 0.99390697f,
  0.99518473f, 0.99631261f, 0.99729046f, 0.99811811f, 0.99879546f, 0.99932238f,
  0.99969882f, 0.99992470f, 1.00000000f, 0.99992470f, 0.99969882f, 0.99932238f,
  0.99879546f, 0.99811811f, 0.99729046f, 0.99631261f, 0.99518473f, 0.99390697f,
  0.99247953f, 0.99090264f, 0.98917651f, 0.98730142f, 0.98527764f, 0.98310549f,
  0.98078528f, 0.97831737f, 0.97570213f, 0.97293995f, 0.97003125f, 0.96697647f,
  0.96377607f, 0.96043052f, 0.95694034f, 0.95330604f, 0.94952818f, 0.94560733f,
  0.94154407f, 0.93733901f, 0.93299280f, 0.92850608f, 0.92387953f, 0.91911385f,
  0.91420976f, 0.90916798f, 0.90398929f, 0.89867447f, 0.89322430f, 0.88763962f,
  0.88192126f, 0.87607009f, 0.87008699f, 0.86397286f, 0.85772861f, 0.85135519f,
  0.84485357f, 0.83822471f, 0.83146961f, 0.82458930f, 0.81758481f, 0.81045720f,
  0.80320753f, 0.79583690f, 0.78834643f, 0.78073723f, 0.77301045f, 0.76516727f,
  0.75720885f, 0.74913639f, 0.74095113f, 0.73265427f, 0.72424708f, 0.71573083f,
  0.70710678f, 0.69837625f, 0.68954054f, 0.68060100f, 0.67155895f, 0.66241578f,
  0.65317284f, 0.64383154f, 0.63439328f, 0.62485949f, 0.61523159f, 0.60551104f,
  0.59569930f, 0.58579786f, 0.57580819f, 0.56573181f, 0.55557023f, 0.54532499f,
  0.53499762f, 0.52458968f, 0.51410274f, 0.50353838f, 0.49289819f, 0.48218377f,
  0.47139674f, 0.46053871f, 0.44961133f, 0.43861624f, 0.42755509f, 0.41642956f,
  0.40524131f, 0.39399204f, 0.38268343f, 0.37131719f, 0.35989504f, 0.34841868f,
  0.33688985f, 0.32531029f, 0.31368174f, 0.30200595f, 0.29028468f, 0.27851969f,
  0.26671276f, 0.25486566f, 0.24298018f, 0.23105811f, 0.21910124f, 0.20711138f,
  0.19509032f, 0.18303989f, 0.17096189f, 0.15885814f, 0.14673047f, 0.13458071f,
  0.12241068f, 0.11022221f, 0.09801714f, 0.08579731f, 0.07356456f, 0.06132074f,
  0.04906767f, 0.03680722f, 0.02454123f, 0.01227154f, 0.00000000f, -0.01227154f,
  -0.02454123f, -0.03680722f, -0.04906767f, -0.06132074f, -0.07356456f,
  -0.08579731f, -0.09801714f, -0.11022221f, -0.12241068f, -0.13458071f,
  -0.14673047f, -0.15885814f, -0.17096189f, -0.18303989f, -0.19509032f, 
  -0.20711138f, -0.21910124f, -0.23105811f, -0.24298018f, -0.25486566f, 
  -0.26671276f, -0.27851969f, -0.29028468f, -0.30200595f, -0.31368174f, 
  -0.32531029f, -0.33688985f, -0.34841868f, -0.35989504f, -0.37131719f, 
  -0.38268343f, -0.39399204f, -0.40524131f, -0.41642956f, -0.42755509f, 
  -0.43861624f, -0.44961133f, -0.46053871f, -0.47139674f, -0.48218377f, 
  -0.49289819f, -0.50353838f, -0.51410274f, -0.52458968f, -0.53499762f, 
  -0.54532499f, -0.55557023f, -0.56573181f, -0.57580819f, -0.58579786f, 
  -0.59569930f, -0.60551104f, -0.61523159f, -0.62485949f, -0.63439328f, 
  -0.64383154f, -0.65317284f, -0.66241578f, -0.67155895f, -0.68060100f, 
  -0.68954054f, -0.69837625f, -0.70710678f, -0.71573083f, -0.72424708f, 
  -0.73265427f, -0.74095113f, -0.74913639f, -0.75720885f, -0.76516727f, 
  -0.77301045f, -0.78073723f, -0.78834643f, -0.79583690f, -0.80320753f, 
  -0.81045720f, -0.81758481f, -0.82458930f, -0.83146961f, -0.83822471f, 
  -0.84485357f, -0.85135519f, -0.85772861f, -0.86397286f, -0.87008699f, 
  -0.87607009f, -0.88192126f, -0.88763962f, -0.89322430f, -0.89867447f, 
  -0.90398929f, -0.90916798f, -0.91420976f, -0.91911385f, -0.92387953f, 
  -0.92850608f, -0.93299280f, -0.93733901f, -0.94154407f, -0.94560733f, 
  -0.94952818f, -0.95330604f, -0.95694034f, -0.96043052f, -0.96377607f, 
  -0.96697647f, -0.97003125f, -0.97293995f, -0.97570213f, -0.97831737f, 
  -0.98078528f, -0.98310549f, -0.98527764f, -0.98730142f, -0.98917651f, 
  -0.99090264f, -0.99247953f, -0.99390697f, -0.99518473f, -0.99631261f, 
  -0.99729046f, -0.99811811f, -0.99879546f, -0.99932238f, -0.99969882f, 
  -0.99992470f, -1.00000000f, -0.99992470f, -0.99969882f, -0.99932238f, 
  -0.99879546f, -0.99811811f, -0.99729046f, -0.99631261f, -0.99518473f, 
  -0.99390697f, -0.99247953f, -0.99090264f, -0.98917651f, -0.98730142f, 
  -0.98527764f, -0.98310549f, -0.98078528f, -0.97831737f, -0.97570213f, 
  -0.97293995f, -0.97003125f, -0.96697647f, -0.96377607f, -0.96043052f, 
  -0.95694034f, -0.95330604f, -0.94952818f, -0.94560733f, -0.94154407f, 
  -0.93733901f, -0.93299280f, -0.92850608f, -0.92387953f, -0.91911385f, 
  -0.91420976f, -0.90916798f, -0.90398929f, -0.89867447f, -0.89322430f, 
  -0.88763962f, -0.88192126f, -0.87607009f, -0.87008699f, -0.86397286f, 
  -0.85772861f, -0.85135519f, -0.84485357f, -0.83822471f, -0.83146961f, 
  -0.82458930f, -0.81758481f, -0.81045720f, -0.80320753f, -0.79583690f, 
  -0.78834643f, -0.78073723f, -0.77301045f, -0.76516727f, -0.75720885f, 
  -0.74913639f, -0.74095113f, -0.73265427f, -0.72424708f, -0.71573083f, 
  -0.70710678f, -0.69837625f, -0.68954054f, -0.68060100f, -0.67155895f, 
  -0.66241578f, -0.65317284f, -0.64383154f, -0.63439328f, -0.62485949f, 
  -0.61523159f, -0.60551104f, -0.59569930f, -0.58579786f, -0.57580819f, 
  -0.56573181f, -0.55557023f, -0.54532499f, -0.53499762f, -0.52458968f, 
  -0.51410274f, -0.50353838f, -0.49289819f, -0.48218377f, -0.47139674f, 
  -0.46053871f, -0.44961133f, -0.43861624f, -0.42755509f, -0.41642956f, 
  -0.40524131f, -0.39399204f, -0.38268343f, -0.37131719f, -0.35989504f, 
  -0.34841868f, -0.33688985f, -0.32531029f, -0.31368174f, -0.30200595f, 
  -0.29028468f, -0.27851969f, -0.26671276f, -0.25486566f, -0.24298018f, 
  -0.23105811f, -0.21910124f, -0.20711138f, -0.19509032f, -0.18303989f, 
  -0.17096189f, -0.15885814f, -0.14673047f, -0.13458071f, -0.12241068f, 
  -0.11022221f, -0.09801714f, -0.08579731f, -0.07356456f, -0.06132074f, 
  -0.04906767f, -0.03680722f, -0.02454123f, -0.01227154f, -0.00000000f
};

void FastSinCos(float x, float *sinVal, float *cosVal)
{
  float fract, in; // Temporary variables for input, output
  unsigned short indexS, indexC; // Index variable
  float f1, f2, d1, d2; // Two nearest output values
  int n;
  float findex, Dn, Df, temp;
  
  // input x is in radians
  //Scale the input to [0 1] range from [0 2*PI] , divide input by 2*pi, for cosine add 0.25 (pi/2) to read sine table
  in = x * 0.159154943092f;
  
  // Calculation of floor value of input
  n = (int) in;
  
  // Make negative values towards -infinity
  if(in < 0.0f){
    n--;
  }
  // Map input value to [0 1]
  in = in - (float) n;
  
  // Calculation of index of the table
  findex = (float) FAST_SIN_TABLE_SIZE * in;
  indexS = ((unsigned short)findex) & 0x1ff;
  indexC = (indexS + (FAST_SIN_TABLE_SIZE / 4)) & 0x1ff;
  
  // fractional value calculation
  fract = findex - (float) indexS;
  
  // Read two nearest values of input value from the cos & sin tables
  f1 = sinTable[indexC+0];
  f2 = sinTable[indexC+1];
  d1 = -sinTable[indexS+0];
  d2 = -sinTable[indexS+1];
  
  Dn = 0.0122718463030f; // delta between the two points (fixed), in this case 2*pi/FAST_SIN_TABLE_SIZE
  Df = f2 - f1; // delta between the values of the functions
  temp = Dn*(d1 + d2) - 2*Df;
  temp = fract*temp + (3*Df - (d2 + 2*d1)*Dn);
  temp = fract*temp + d1*Dn;
  
  // Calculation of cosine value
  *cosVal = fract*temp + f1;
  
  // Read two nearest values of input value from the cos & sin tables
  f1 = sinTable[indexS+0];
  f2 = sinTable[indexS+1];
  d1 = sinTable[indexC+0];
  d2 = sinTable[indexC+1];
  
  Df = f2 - f1; // delta between the values of the functions
  temp = Dn*(d1 + d2) - 2*Df;
  temp = fract*temp + (3*Df - (d2 + 2*d1)*Dn);
  temp = fract*temp + d1*Dn;
  
  // Calculation of sine value
  *sinVal = fract*temp + f1;
}

float FastSin(float x)
{
  float sinVal, fract, in; // Temporary variables for input, output
  unsigned short index; // Index variable
  float a, b; // Two nearest output values
  int n;
  float findex;
  
  // input x is in radians
  // Scale the input to [0 1] range from [0 2*PI] , divide input by 2*pi
  in = x * 0.159154943092f;
  
  // Calculation of floor value of input
  n = (int) in;
  
  // Make negative values towards -infinity
  if(x < 0.0f){
    n--;
  }
  
  // Map input value to [0 1]
  in = in - (float) n;
  
  // Calculation of index of the table
  findex = (float) FAST_SIN_TABLE_SIZE * in;
  index = ((unsigned short)findex) & 0x1ff;
  
  // fractional value calculation
  fract = findex - (float) index;
  
  // Read two nearest values of input value from the sin table
  a = sinTable[index];
  b = sinTable[index+1];
  
  // Linear interpolation process
  sinVal = (1.0f-fract)*a + fract*b;
  
  // Return the output value
  return (sinVal);
}

float FastCos(float x)
{
  float cosVal, fract, in; // Temporary variables for input, output
  unsigned short index; // Index variable
  float a, b; // Two nearest output values
  int n;
  float findex;
  
  // input x is in radians
  // Scale the input to [0 1] range from [0 2*PI] , divide input by 2*pi, add 0.25 (pi/2) to read sine table
  in = x * 0.159154943092f + 0.25f;
  
  // Calculation of floor value of input
  n = (int) in;
  
  // Make negative values towards -infinity
  if(in < 0.0f){
    n--;
  }
  
  // Map input value to [0 1]
  in = in - (float) n;
  
  // Calculation of index of the table
  findex = (float) FAST_SIN_TABLE_SIZE * in;
  index = ((unsigned short)findex) & 0x1ff;
  
  // fractional value calculation
  fract = findex - (float) index;
  
  // Read two nearest values of input value from the cos table
  a = sinTable[index];
  b = sinTable[index+1];
  
  // Linear interpolation process
  cosVal = (1.0f-fract)*a + fract*b;
  
  // Return the output value
  return (cosVal);
}


