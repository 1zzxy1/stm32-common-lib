#ifndef __WP_MATH_H
#define __WP_MATH_H

#include "stdint.h"

#define AcceMax     2048 
#define AcceGravity 9.80f
/*************WGS84地心坐标参考系数**************/
#define WGS84_RADIUS_EQUATOR        6378137.0f//半长轴，单位m
#define WGS84_INVERSE_FLATTENING    298.257223563f//扁率
#define WGS84_FLATTENING            (1/WGS84_INVERSE_FLATTENING)//扁率导数
#define WGS84_RADIUS_POLAR          (WGS84_RADIUS_EQUATOR*(1-WGS84_FLATTENING))//短轴
#define WGS84_ECCENTRICITY_SQUARED  (2*WGS84_FLATTENING-WGS84_FLATTENING*WGS84_FLATTENING)
/*********************************************
经度方向距离：LON_TO_CM*经度差，LON_TO_CM对应武汉地区所在纬度平面圆周长
纬度方向距离：LAT_TO_CM*纬度差，
*********************************************/

#define LON_COSINE_LOCAL 0.860460f//约等于当地纬度的余弦值，cos(Lat*DEG_TO_RAD)
//#define LAT_TO_CM  (2.0f * WGS84_RADIUS_EQUATOR * PI / (360.0f * 100000.0f))
//#define LON_TO_CM  (2.0f * WGS84_RADIUS_EQUATOR * PI / (360.0f * 100000.0f))*LON_COSINE_LOCAL
#define LAT_TO_CM  (2.0f * WGS84_RADIUS_EQUATOR * PI/360.0f)*100.0f
#define LON_TO_CM  (2.0f * WGS84_RADIUS_EQUATOR * PI*LON_COSINE_LOCAL/360.0f)*100.0f

#define LAT_TO_M  (2.0f * WGS84_RADIUS_EQUATOR * PI/360.0f)
#define LON_TO_M  (2.0f * WGS84_RADIUS_EQUATOR * PI*LON_COSINE_LOCAL/360.0f)

#ifndef M_PI_F
 #define M_PI_F 3.141592653589793f
#endif
#ifndef PI
 # define PI M_PI_F
#endif
#ifndef M_PI_2
 # define M_PI_2 1.570796326794897f
#endif
//Single precision conversions
#define DEG_TO_RAD 0.017453292519943295769236907684886f
#define RAD_TO_DEG 57.295779513082320876798154814105f

// radius of earth in meters
#define RADIUS_OF_EARTH 6378100
// scaling factor from 1e-7 degrees to meters at equater
// == 1.0e-7 * DEG_TO_RAD * RADIUS_OF_EARTH
#define LOCATION_SCALING_FACTOR 0.011131884502145034f
// inverse of LOCATION_SCALING_FACTOR
#define LOCATION_SCALING_FACTOR_INV 89.83204953368922f


float safe_asin(float v);
float safe_sqrt(float v);
float fast_atan(float v);
float fast_atan2(float y, float x);
float constrain_float(float amt, float low, float high);
int16_t constrain_int16(int16_t amt, int16_t low, int16_t high);
int32_t constrain_int32(int32_t amt, int32_t low, int32_t high);
float radians(float deg);
float degrees(float rad);
float sq(float v);
float pythagorous2(float a, float b);
float pythagorous3(float a, float b, float c);




#define ROOT_HALF (0.70710678118654752440084436210485f)
#define LOGA_COEF0 (-4.649062303464e-1f)
#define LOGA_COEF1 (+1.360095468621e-2f)
#define LOGB_COEF0 (-5.578873750242f)

#define LOGDA_COEF0 (-6.4124943423745581147e1f)
#define LOGDA_COEF1 (+1.6383943563021534222e1f)
#define LOGDA_COEF2 (-7.8956112887491257267e-1f)
#define LOGDB_COEF0 (-7.6949932108494879777e2f)
#define LOGDB_COEF1 (+3.1203222091924532844e2f)
#define LOGDB_COEF2 (-3.5667977739034646171e1f)
#define LN2_DC1 (0.693359375f)
#define LN2_DC2 (-2.121944400e-4f)
#define LN2_DC3 (-5.46905827678e-14f)

float FastLn(float x);
//////////////////////////////////////////////////////////////////////////
///Coefficients used for pow
#define POWP_COEF1    (+8.33333286245e-2f)
#define POWP_COEF2    (+1.25064850052e-2f)
#define POWQ_COEF1    (+6.93147180556341e-1f)
#define POWQ_COEF2    (+2.40226506144710e-1f)
#define POWQ_COEF3    (+5.55040488130765e-2f)
#define POWQ_COEF4    (+9.61620659583789e-3f)
#define POWQ_COEF5    (+1.30525515942810e-3f)

#define POW_BIGNUM (+2046.0f)
#define POW_SMALLNUM (-2015.0f)

#define LOG2E_MINUS1 (0.44269504088896340735992468100189f)
//
#define FLT_EPSILON          1.1920928955078125E-07F 
#define FLT_MAX              3.4028234663852886E+38F
//
float FastPow(float x,float y);
//////////////////////////////////////////////////////////////////////////
//
#define X_MAX (+9.099024257348e3f)
#define INV_PI_2 ( 0.63661977236758134307553505349006f)
#define PI_2_C1             ( 1.5703125f)
#define PI_2_C2             ( 4.84466552734375e-4f)
#define PI_2_C3 (-6.39757837755768678308360248557e-7f)

#define TANP_COEF1    (-1.113614403566e-1f)
#define TANP_COEF2    (+1.075154738488e-3f)
#define TANQ_COEF0    (+1.000000000000f)
#define TANQ_COEF1    (-4.446947720281e-1f)
#define TANQ_COEF2    (+1.597339213300e-2f)
float FastTan(float x);
//////////////////////////////////////////////////////////////////////////

#define _2_PI 6.283185307179586476925286766559f
#define RADTODEG(x) ((x) * 57.295779513082320876798154814105f)
#define DEGTORAD(x) ((x) * 0.01745329251994329576923690768489f)

//translate from the DSP instruction of a DSP Library.
#ifndef PI
#define PI (3.1415926535897932384626433832795f)
#endif
#define PI_2 (1.5707963267948966192313216916398f)
#define PI_3 (1.0471975511965977461542144610932f)
#define PI_4 (0.78539816339744830961566084581988f)
#define PI_6 (0.52359877559829887307710723054658f)
#define TWO_MINUS_ROOT3 (0.26794919243112270647255365849413f)
#define SQRT3_MINUS_1 (0.73205080756887729352744634150587f)
#define SQRT3 (1.7320508075688772935274463415059f)
#define EPS_FLOAT (+3.452669830012e-4f)
//Coefficients used for atan/atan2
#define ATANP_COEF0 (-1.44008344874f)
#define ATANP_COEF1 (-7.20026848898e-1f)
#define ATANQ_COEF0 (+4.32025038919f)
#define ATANQ_COEF1 (+4.75222584599f)
//Coefficients used for asin/acos
#define ASINP_COEF1 (-2.7516555290596f)
#define ASINP_COEF2 (+2.9058762374859f)
#define ASINP_COEF3 (-5.9450144193246e-1f)
#define ASINQ_COEF0 (-1.6509933202424e+1f)
#define ASINQ_COEF1 (+2.4864728969164e+1f)
#define ASINQ_COEF2 (-1.0333867072113e+1f)

float FastAsin(float x);
float FastAtan2(float y, float x);

float FastSqrtI(float x);
float FastSqrt(float x);

float FastSin(float x);
float FastCos(float x);
void FastSinCos(float x, float *sinVal, float *cosVal);

inline float FastAbs(float x){
	union { unsigned int i; float f;} y;
	y.f = x;
	y.i = y.i & 0x7FFFFFFF;
	return (float)y.f;
}

//////////////////////////////////////////////////////////////////////////


#endif

