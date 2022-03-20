/*
 * @Author: xiao jia
 * @email: limu.xiao@tuya.com
 * @Date: 2021-09-13 11:24:33
 * @file name:
 * @Description:
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 */

#include "light_types.h"
#include "light_tools.h"

#define SET_BIT(value,bit) (value|=(1<<bit))
#define CLR_BIT(value,bit) (value&=~(1<<bit))
#define GET_BIT(value,bit) (value&(1<<bit))

#define MIN3(X,Y,Z)     ((Y) <= (Z) ? ((X) <= (Y) ? (X) : (Y)) : ((X) <= (Z) ? (X) : (Z)))
#define MAX3(X,Y,Z)     ((Y) >= (Z) ? ((X) >= (Y) ? (X) : (Y)) : ((X) >= (Z) ? (X) : (Z)))


float rgb_2_xyz[3][3];  //conversion Matrix
float xyz_2_rgb[3][3];

/******************************************************************************************************
FUCTIONS
******************************************************************************************************/
USHORT_T zig_b_2_app_B(UCHAR_T zig_b)
{
    USHORT_T result;
    result = (USHORT_T)((float)zig_b * APP_WHITE_B_MAX / ZIGBEE_WHITE_B_MAX + 0.5);
    if(result > APP_WHITE_B_MAX ){
        result = APP_WHITE_B_MAX;
    }
    return result;
}

UCHAR_T app_B_2_zig_b(USHORT_T app_B)
{
    UCHAR_T result;
    result = (UCHAR_T)((float)app_B *  ZIGBEE_WHITE_B_MAX / APP_WHITE_B_MAX + 0.5);
    return result;
}

USHORT_T zig_h_2_app_H(UCHAR_T zig_h)
{
    USHORT_T result = (USHORT_T)((float)zig_h * APP_COLOR_H_MAX / ZIGBEE_COLOR_H_MAX + 0.5);
    if(result > APP_COLOR_H_MAX){
        result = APP_COLOR_H_MAX;
    }
    return result;
}

UCHAR_T app_H_2_zig_h(USHORT_T app_H)
{
    UCHAR_T result;
    result = (UCHAR_T)((float)app_H * ZIGBEE_COLOR_H_MAX / APP_COLOR_H_MAX + 0.5);
    if(result > ZIGBEE_COLOR_H_MAX){
        result = ZIGBEE_COLOR_H_MAX;
    }
    return result;
}

USHORT_T zig_s_2_app_S(UCHAR_T zig_s)
{
    USHORT_T result = (USHORT_T)((float)zig_s * APP_COLOR_S_MAX / ZIGBEE_COLOR_S_MAX + 0.5);
    if(result > APP_COLOR_S_MAX){
        result = APP_COLOR_S_MAX;
    }
    return result;
}

UCHAR_T app_S_2_zig_s(USHORT_T app_S)
{
    UCHAR_T result;
    result = (UCHAR_T)((float)app_S * ZIGBEE_COLOR_H_MAX / APP_COLOR_S_MAX + 0.5);
    if(result > ZIGBEE_COLOR_S_MAX){
        result = ZIGBEE_COLOR_S_MAX;
    }
    return result;
}

/**
 * @description: compare two string
 * @param {IN CHAR_T* str1 -> string 1}
 * @param {IN CHAR_T* str2 -> string 2}
 * @return: 0: if these two string is not same,
 *          1: these two string is all same
 */
BOOL_T b_string_compare(CHAR_T* str1, CHAR_T* str2)
{

    while(*str1 !='\0'&& *str2 != '\0')
    {
        if(*str1 != *str2){
            return 0;
        }
        str1++;
        str2++;
    }

    if(*str1 =='\0'&& *str2 == '\0') {
        return 1;
    } else {
      return 0;
    }
}

/**
 * @berief: get the absolute value
 * @param { INT_T value -> calc value }
 * @return: absolute value
 * @retval: none
 */
UINT_T light_tool_get_abs_value(INT_T value)
{
    return (value > 0 ? value : -value);
}

/**
 * @berief: get the max value in 3 numbers
 * @param { UINT_T a -> number1 }
 * @param { UINT_T b -> number2 }
 * @param { UINT_T c -> number3 }
 * @return: Max value
 * @retval: none
 */
UINT_T light_tool_get_max_value(UINT_T a, UINT_T b, UINT_T c)
{
    UINT_T x = a > b ? a : b;
    UINT_T y = x > c ? x : c;

    return y;
}


/**
 * @berief: HSV change to RGB
 * @param {IN USHORT_T h -> range 0~360 }
 * @param {IN USHORT_T s -> range 0~1000}
 * @param {IN USHORT_T v -> range 0~1000}
 * @param {OUT USHORT_T *r -> R result,rang from 0~1000}
 * @param {OUT USHORT_T *g -> G result,rang from 0~1000}
 * @param {OUT USHORT_T *b -> B result,rang from 0~1000}
 * @return: none
 * @retval: none
 */
void light_tool_HSV_2_RGB(USHORT_T h, USHORT_T s, USHORT_T v, USHORT_T *r, USHORT_T *g, USHORT_T *b)
{
    INT_T i;
    FLOAT_T RGB_min, RGB_max;
    FLOAT_T RGB_Adj;
    INT_T difs;

    if(h >= 360) {
        h = 0;
    }

    RGB_max = v * 1.0f;
    RGB_min = RGB_max*(1000 - s) / 1000.0f;

    i = h / 60;
    difs = h % 60; /* factorial part of h */


    /* RGB adjustment amount by hue */
    RGB_Adj = (RGB_max - RGB_min)*difs / 60.0f;

    switch (i) {
        case 0:
            *r = (USHORT_T)RGB_max;
            *g = (USHORT_T)(RGB_min + RGB_Adj);
            *b = (USHORT_T)RGB_min;
            break;

        case 1:
            *r = (USHORT_T)(RGB_max - RGB_Adj);
            *g = (USHORT_T)RGB_max;
            *b = (USHORT_T)RGB_min;
            break;

        case 2:
            *r = (USHORT_T)RGB_min;
            *g = (USHORT_T)RGB_max;
            *b = (USHORT_T)(RGB_min + RGB_Adj);
            break;

        case 3:
            *r = (USHORT_T)RGB_min;
            *g = (USHORT_T)(RGB_max - RGB_Adj);
            *b = (USHORT_T)RGB_max;
            break;

        case 4:
            *r = (USHORT_T)(RGB_min + RGB_Adj);
            *g = (USHORT_T)RGB_min;
            *b = (USHORT_T)RGB_max;
            break;

        default:        // case 5:
            *r = (USHORT_T)RGB_max;
            *g = (USHORT_T)RGB_min;
            *b = (USHORT_T)(RGB_max - RGB_Adj);
            break;
    }

    if(*r > 1000) {
        *r = 1000;
    }

    if(*g > 1000) {
        *g = 1000;
    }

    if(*b > 1000) {
        *b = 1000;
    }
}

/************************************************************************
 * DESCRIPTION: Checks is a floating point number is out of range
 ***********************************************************************/
static bool_t checkNumberIsValid(float fValue)
{
    short exp2;
    union
    {
        long    l;
        float   f;
    } uFL;
    uFL.f = fValue;
    exp2 = (unsigned char)(uFL.l >> 23) - 127;
    if (exp2 >= 31)
    {
        /* Number is too large */
        return false;
    }
    else if (exp2 < -23)
    {
        /* Number is too small */
        return false;
    }
    return true;
}

/****************************************************************************
 ** PARAMETERS:                 Name                        Usage
 ** float                       fx                          x value
 ** float                       fy                          y value
 ** float                       fY                          Y value
 ** float                       *pfX                        Result X
 ** float                       *pfY                        Result Y
 ** float                       *pfZ                        Result Z
 ****************************************************************************/
static bool_t bulbxyY2XYZFloat(
        float    fx,
        float    fy,
        float    fY,
        float    *pfX,
        float    *pfY,
        float    *pfZ)
{
    /* z = 1 - x - y */
    float fz = (float)(1.0 - fx - fy);
    /* X = (x / y) * Y */
    *pfX = (fx / fy) * fY;
    /* Y = Y */
    *pfY = fY;
    /* Z = (z / y) * Y */
    *pfZ = (fz / fy) * fY;


    /* Check the results are valid */
    if((checkNumberIsValid(*pfX) == false) ||
       (checkNumberIsValid(*pfY) == false) ||
       (checkNumberIsValid(*pfZ) == false))
    {
        return false;
    }
    return true;
}


/***********************************************************************
* PARAMETERS:                Name                        Usage
 ** float                    afMatrix                    3x3 conversion matrix
 ** float                      fX                          X value
 ** float                      fY                          Y value
 ** float                      fZ                          Z value
 ** float                      *pfRed                      Result Red
 ** float                      *pfGreen                    Result Green
 ** float                      *pfBlue                     Result Blue
 ************************************************************************/
static void bulbXYZ2RGBFloat(
        float     afMatrix[3][3],
        float     fX,
        float     fY,
        float     fZ,
        float     *pfRed,
        float     *pfGreen,
        float     *pfBlue)
{

    float R, G, B, Max;

    R = fX * afMatrix[0][0] + fY * afMatrix[0][1] + fZ * afMatrix[0][2];
    G = fX * afMatrix[1][0] + fY * afMatrix[1][1] + fZ * afMatrix[1][2];
    B = fX * afMatrix[2][0] + fY * afMatrix[2][1] + fZ * afMatrix[2][2];

    Max = MAX3(R, G, B);

    if(Max > 1.000){
        R = R / Max;
        G = G / Max;
        B = B / Max;
    }

    if(R < 0.0) R = 0;
    if(G < 0.0) G = 0;
    if(B < 0.0) B = 0;

    *pfRed      = R;
    *pfGreen    = G;
    *pfBlue     = B;
}


/****************************************************************************
 ** PARAMETERS:                 Name                        Usage
 ** float                       cX                         currentX value
 ** float                       cY                         currentY value
 ** float                       fBlue                       blue value
 ** float                       *pfx                        Result x
 ** float                       *pfy                        Result y
 ** float                       *pfY                        Result Y
****************************************************************************/
static bool_t bulbxyY2RGBInt(
        USHORT_T  cX,
        USHORT_T  cY,
        UCHAR_T   *pu8Red,
        UCHAR_T   *pu8Green,
        UCHAR_T   *pu8Blue)
{

    float x, y, BigY;
    float X, Y, Z;
    float R, G, B;
    /* Convert xyY to floating point values */
    x    = (float)((float)cX / 65535.0);
    y    = (float)((float)cY / 65535.0);
    BigY = 1.0;
    /* Convert xyY to XYZ colour space */
    if(bulbxyY2XYZFloat(x, y, BigY, &X, &Y, &Z) != true)
    {
        return false;
    }
    /* Convert XYZ to RGB colour space */
    bulbXYZ2RGBFloat(xyz_2_rgb, X, Y, Z, &R, &G, &B);
    *pu8Red     = (UCHAR_T)(255.0 * R);
    *pu8Green   = (UCHAR_T)(255.0 * G);
    *pu8Blue    = (UCHAR_T)(255.0 * B);
    return true;
}

/**********************************************************************
 ** PARAMETERS:                 Name                        Usage
 ** float                       afMatrix                    Pointer to 3x3 conversion matrix
 ** float                       fRed                        Red value(0~255)
 ** float                       fGreen                      Green value(0~255)
 ** float                       fBlue                       Blue value (0~255)
 ** float                       *pfX                        Result X
 ** float                       *pfY                        Result Y
 ** float                       *pfZ                        Result Z
 ****************************************************************************/
void bulbRGB2XYZFloat(
                float   afMatrix[3][3],
                float   fRed,
                float   fGreen,
                float   fBlue,
                float   *pfX,
                float   *pfY,
                float   *pfZ)
{
    *pfX = fRed * afMatrix[0][0] + fGreen * afMatrix[0][1] + fBlue * afMatrix[0][2];
    *pfY = fRed * afMatrix[1][0] + fGreen * afMatrix[1][1] + fBlue * afMatrix[1][2];
    *pfZ = fRed * afMatrix[2][0] + fGreen * afMatrix[2][1] + fBlue * afMatrix[2][2];
}


/****************************************************************************
 ** PARAMETERS:                 Name                        Usage
 ** float                       fX                          X value
 ** float                       fY                          Y value
 ** float                       fZ                          Z value
 ** float                       *pfx                        Result x
 ** float                       *pfy                        Result y
 ** float                       *pfY                        Result Y
****************************************************************************/
bool_t bulbXYZ2xyYFloat(
        float      fX,
        float      fY,
        float      fZ,
        float      *pfx,
        float      *pfy,
        float      *pfY)
{

    /* x = X / (X + Y + Z) */
    *pfx = (fX / (fX + fY + fZ));

    /* y = Y / (X + Y + Z) */
    *pfy = (fY / (fX + fY + fZ));

    /* Y = Y */
    *pfY = fY;

    /* Check the results are valid */
    if((checkNumberIsValid(*pfx) == false) ||
       (checkNumberIsValid(*pfy) == false) ||
       (checkNumberIsValid(*pfY) == false))
    {
        return false;
    }
    else
    {
       return true;
    }

}

static void bulbRGB2xyYInt(  
                  UCHAR_T   fRed,
                  UCHAR_T   fGreen,
                  UCHAR_T   fBlue,
                  USHORT_T  *pu16x,
                  USHORT_T  *pu16y)
{
  float   R,G,B;
  float   X,Y,Z;
  float   x, y, BigY;
  R = fRed;
  G = fGreen;
  B = fBlue;
  
  bulbRGB2XYZFloat(rgb_2_xyz, R, G, B, &X, &Y, &Z);
  
  bulbXYZ2xyYFloat(X, Y, Z, &x, &y, &BigY);
  
  *pu16x = (USHORT_T)(65535.0 * x);
  *pu16y = (USHORT_T)(65535.0 * y);
  //*pu8Y  = (UCHAR_T)(255.0 * BigY);
  // return true;
}

/**
 * @description: RGB to XY
 * @param {r} 0~1000 range
 * @param {g} 0~1000 range
 * @param {b} 0~1000 range
 * @param {x} 0~1000 range
 * @param {y} 0~1000 range
 * @return: none
 */
void vLightToolRGB2XY(USHORT_T r, USHORT_T g, USHORT_T b, USHORT_T *x, USHORT_T *y)
{
    USHORT_T R,G,B;
    R = (UCHAR_T)((float)r * 255 / 1000 + 0.5);
    G = (UCHAR_T)((float)g * 255 / 1000 + 0.5);
    B = (UCHAR_T)((float)b * 255 / 1000 + 0.5);

    bulbRGB2xyYInt(R,G,B,x,y);
}

/**
 * @description: XY convert to RGB
 * @param {IN USHORT_T X} current X
 * @param {IN USHORT_T Y} current Y
 * @param {IN USHORT_T level} current level
 * @param {IN USHORT_T *r} out R
 * @param {IN USHORT_T *g} out G
 * @param {IN USHORT_T *b} out B
 * @return: none
 */
BOOL_T light_tool_XY_2_RGB(USHORT_T X, USHORT_T Y, USHORT_T level, USHORT_T *r, USHORT_T *g, USHORT_T *b)
{
    BOOL_T ret;
    UCHAR_T R,G,B;

    float temp =  level / 255.0;

    ret = bulbxyY2RGBInt(X,Y,&R,&G,&B);
    if(ret != TRUE){
        PR_ERR("SET XY error");
        return FALSE;
    }
    *r = (USHORT_T)(R) * temp;
    *g = (USHORT_T)(G) * temp;
    *b = (USHORT_T)(B) * temp;
    return TRUE;
}

/**
 * @description: HSV to XY
 * @param {h} 0~360 range
 * @param {S} 0~1000 range
 * @param {V} 0~1000 range
 * @param {x} CIE x
 * @param {y} CIE y
 * @return {type} none
 */
void vLightToolHSV2XY(USHORT_T H, USHORT_T S, USHORT_T V, USHORT_T *X, USHORT_T *Y)
{
    USHORT_T R, G, B;
    light_tool_HSV_2_RGB(H, S, V, &R, &G, &B);
    vLightToolRGB2XY(R,G,B,X,Y);
}

/**
 * @description: XY to HSV
 *  @param {x} CIE x
 * @param {y} CIE y
 * @param {h} 0~360 range
 * @param {S} 0~1000 range
 * @param {V} 0~1000 range
 * @return {type} none
 */
void vLightToolXY2HSV(USHORT_T X, USHORT_T Y, USHORT_T Level, USHORT_T *H, USHORT_T *S, USHORT_T *V)
{
    USHORT_T R, G, B;
    light_tool_XY_2_RGB(X, Y, Level, &R, &G, &B);
    light_tool_RGB_2_HSV(R, G, B, H, S, V);
}



FLOAT_T fLightToolGetMAX(FLOAT_T a, FLOAT_T b)
{
    if(a >= b) {
        return a;
    } else {
        return b;
    }
}

FLOAT_T fLightToolGetMIN(FLOAT_T a, FLOAT_T b)
{
    if(a <= b) {
        return a;
    } else {
        return b;
    }
}

/**
 * @berief: ASSIC change to hex
 * @param { CHAR_T asc_code -> ASSIC code }
 * @return: hex value
 * @retval: none
 */
UCHAR_T light_tool_ASC_2_HEX(CHAR_T asc_code)
{
    UCHAR_T ucResult = 0;

    if( '0' <= asc_code && asc_code <= '9' ) {
        ucResult = asc_code - '0';
    } else if ( 'a' <= asc_code && asc_code <= 'f' ) {
        ucResult = asc_code - 'a' + 10;
    } else if ( 'A' <= asc_code && asc_code <= 'F' ) {
        ucResult = asc_code - 'A' + 10;
    } else {
        ucResult = 0;
    }

    return ucResult;
}

/**
 * @berief: four unsigned char merge into unsigned short
 * @param { UCHAR_T HH -> USHORT Hight hight 4bit }
 * @param { UCHAR_T HL -> USHORT Hight low 4bit   }
 * @param { UCHAR_T LH -> USHORT low hight 4bit   }
 * @param { UCHAR_T LL -> USHORT low low 4bit     }
 * @return: USHORT value
 * @retval: none
 */
USHORT_T light_tool_STR_2_USHORT(UCHAR_T HH, UCHAR_T HL, UCHAR_T LH, UCHAR_T LL)
{
    return ( (HH << 12) | (HL << 8) | (LH << 4) | (LL & 0x0F) );
}

/**
 * @berief: RGB change to HSV
 * @param {IN USHORT_T R -> R,rang from 0~1000}
 * @param {IN USHORT_T G -> G,rang from 0~1000}
 * @param {IN USHORT_T B -> B,rang from 0~1000}
 * @param {OUT USHORT_T H -> result, range 0~360}
 * @param {OUT USHORT_T S -> result, range 0~1000}
 * @param {OUT USHORT_T V -> result, range 0~1000}
 * @return: none
 * @retval: none
 */
void light_tool_RGB_2_HSV(USHORT_T R, USHORT_T G, USHORT_T B, USHORT_T *H, USHORT_T *S, USHORT_T *V)
{
    FLOAT_T r,g,b;
    FLOAT_T min_rgb,max_rgb,delta_rgb;
    FLOAT_T h,s,v;

    r = R / 1000.0f;
    g = G / 1000.0f;
    b = B / 1000.0f;

    min_rgb = fLightToolGetMIN(r,fLightToolGetMIN(g,b));
    max_rgb = fLightToolGetMAX(r,fLightToolGetMAX(g,b));

    delta_rgb = max_rgb - min_rgb;

    v = max_rgb;
    if(max_rgb != 0.0) {
        s = delta_rgb / max_rgb;
    } else {
        s = 0.0;
    }

    if(s <= 0.0) {
        h = -1.0f;
    } else {
        if(r == max_rgb) {
            h = (g-b) / delta_rgb;
        } else if (g == max_rgb) {
            h = 2.0 + (b-r) / delta_rgb;
        } else if (b == max_rgb) {
            h = 4.0 + (r-g) / delta_rgb;
        }
    }

    h = h * 60.0;
    if(h < 0.0) {
        h += 360;
    }

    *H = (USHORT_T)h;
    *S = (USHORT_T)(s * 1000);
    *V = (USHORT_T)(v * 1000);
}


/**
 * @description: compress one uint scene data,from string (26 bytes) to hex (65 bits).
 * compress format:
 * bits: mode:2 bits, time1:7bits, timer2:7bits, H:9bits, S:10 bits, V:10 bits,B:10bits, T:10bits;
 * output_buf[0]: mode_1 + time1
 * output_buf[1]: H_1 + time2
 * output_buf[2]: H_8
 * output_buf[3]: S_2 + V_2 + B_2 + T_2
 * output_buf[4]: S_8
 * output_buf[5]: V_8
 * output_buf[6]: B_8
 * output_buf[7]: T_8
 * @param {input_str} one uint data of scene
 * @param {output_buf} Output buffer address,a unit of compressed data,including speed,transition time,H,S,V,B,T
 * @param {light_mode} output transition mode of the compressed data
 * @return: OPERATE_RET
 */
STATIC OPERATE_RET op_scene_data_compressToOneUint(CHAR_T* input_str, UCHAR_T *output_buf , UCHAR_T *light_mode)
{
    if(NULL == input_str || NULL == output_buf || strlen(input_str) < 26){
        return OPRT_INVALID_PARM;
    }

    INT_T offset = 0;
    CHAR_T tmp_str[5] = {0};

    //unit  time
    strncpy(tmp_str, input_str + offset, 2);
    USHORT_T time1 = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 2;

    //unit stand time
    memset(tmp_str, 0, 5);
    strncpy(tmp_str, input_str + offset, 2);
    USHORT_T time2 = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 2;

    //unit change mode
    memset(tmp_str, 0, 5);
    strncpy(tmp_str, input_str + offset, 2);
    USHORT_T mode = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 2;

    //Hue
    memset(tmp_str, 0, 5);
    strncpy(tmp_str, input_str + offset, 4);
    USHORT_T val_H = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 4;

    //Saturation
    memset(tmp_str, 0, 5);
    strncpy(tmp_str, input_str + offset, 4);
    USHORT_T val_S = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 4;

    //Value
    memset(tmp_str, 0, 5);
    strncpy(tmp_str, input_str + offset, 4);
    USHORT_T val_V = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 4;

    //Bright
    memset(tmp_str, 0, 5);
    strncpy(tmp_str, input_str + offset, 4);
    USHORT_T val_L = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 4;

    //Temperature
    memset(tmp_str, 0, 5);
    strncpy(tmp_str, input_str + offset, 4);
    USHORT_T val_T = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 4;
    output_buf[0] = ((mode & 0x1) << 7) | (time1 & 0x7F);
    output_buf[1] = (((val_H >> 8) & 0x1) << 7) | (time2 & 0x7F);
    output_buf[2] = (val_H & 0xFF);
    output_buf[3] = (((val_S >> 8) & 0x3) << 6) | (((val_V >> 8) & 0x3) << 4) | (((val_L >> 8) & 0x3) << 2) | ((val_T >> 8) & 0x3);
    output_buf[4] = (val_S & 0xFF);
    output_buf[5] = (val_V & 0xFF);
    output_buf[6] = (val_L & 0xFF);
    output_buf[7] = (val_T & 0xFF);

    *light_mode = mode;

    return OPRT_OK;
}

/**
 * @description: compress scene data,from string (max 2+26*8=210 bytes) to hex (8+8+64*8)bits = 66 bytes).
 * compressed data format:
 * scene id + mode + uint*8
 * @param {input_str} scene data string format
 * @param {output_buf} Output buffer address, compressed data.
 * @param {output_buf_len} output compressed data length
 * @return: 1:success / 0:failed
 */
OPERATE_RET op_scene_data_compress(CHAR_T* input_str, UCHAR_T* output_buf, UINT_T *output_buf_len)
{
    OPERATE_RET opRet = -1;
    INT_T str_len ;
    INT_T i = 0;
    UCHAR_T light_mode_tmp = 0;
    CHAR_T tmp_str[5] = {0};
    USHORT_T num;

    if(input_str == NULL){
        return OPRT_INVALID_PARM;
    }

    str_len = strlen(input_str);
    if((0 != (str_len % 2)) || (2 != (str_len % 26))) {
        return -1;
    }

    for(i = 0; (i * 26 + 2) < str_len; i++) {
        opRet = op_scene_data_compressToOneUint(input_str + 2 + i * 26, output_buf + 2 + i * 8, &light_mode_tmp);
        if(opRet != OPRT_OK) {
            PR_ERR("scene compress error!");
        }
        output_buf[1] |= (((light_mode_tmp >> 1) & 0x1) << i);
        light_mode_tmp = 0;
    }

    //fill scene number
    strncpy(tmp_str, input_str, 2);
    num = (USHORT_T) strtol(tmp_str, NULL, 16);
    output_buf[0] =  (num & 0xFF);

    //calc scene length
    *output_buf_len = i * 8 + 2;

    return OPRT_OK;
}

/**
 * @description: decompress one uint scene data, to string format
 * @param {input_buf} scene compressed data
 * @param {light_mode} mode value
 * @param {output_str} output compressed data length
 * @return: OPERATE_RET
 */
OPERATE_RET opDecompressSceneDataOneUint(UCHAR_T *input_buf, UCHAR_T light_mode, CHAR_T * output_str)
{
    CHAR_T output_str_tmp[27] = {0};

    if(NULL == input_buf || NULL == output_str) {
        return OPRT_INVALID_PARM;
    }

    USHORT_T mode = ((light_mode & 0x1) << 1) | ((input_buf[0] >> 7) & 0x1);
    USHORT_T time1 = (input_buf[0] & 0x7F);
    USHORT_T time2 = (input_buf[1] & 0x7F);
    USHORT_T val_H = (((input_buf[1] >> 7) & 0x1) << 8) | (input_buf[2] & 0xFF);
    USHORT_T val_S = (input_buf[4] & 0xFF) | (((input_buf[3] >> 6) & 0x3) << 8);
    USHORT_T val_V = (input_buf[5] & 0xFF) | (((input_buf[3] >> 4) & 0x3) << 8);
    USHORT_T val_L = (input_buf[6] & 0xFF) | (((input_buf[3] >> 2) & 0x3) << 8);
    USHORT_T val_T = (input_buf[7] & 0xFF) | ((input_buf[3] & 0x3) << 8);

    memset(output_str_tmp, 0, 27);
    snprintf(output_str_tmp, 27, "%02x%02x%02x%04x%04x%04x%04x%04x", time1, time2, mode, val_H, val_S, val_V, val_L, val_T);
    strncpy(output_str, output_str_tmp, 26);

    return OPRT_OK;
}

/**
 * @description: decompress scene data to string format
 * @param {input_buf} compressed scene data
 * @param {input_buf_len} compressed scene data length
 * @param {output_str} decompressed scene data,string format
 * @return: OPERATE_RET
 */
OPERATE_RET opSceneDataDecompress(UCHAR_T* input_buf, UINT_T input_buf_len, CHAR_T* output_str)
{
    OPERATE_RET opRet = -1;

	INT_T i = 0;
    CHAR_T output_str_tmp[27] = {0};
    UCHAR_T light_mode_tmp = 0;

    if(NULL == input_buf || NULL == output_str || 2 != (input_buf_len % 8)) {
        return OPRT_INVALID_PARM;
    }

    //fill scene number
    snprintf(output_str, 3, "%02x", input_buf[0]);

    //fill scene unit
    for(i = 0; (i * 8 + 2) < input_buf_len; i++) {
        memset(output_str_tmp, 0, 27);
        light_mode_tmp = ((input_buf[1] >> i) & 0x1);
        opRet = opDecompressSceneDataOneUint(input_buf + 2 + (i * 8), light_mode_tmp, output_str_tmp);
        if(opRet != OPRT_OK) {
            PR_ERR("scene decompress error!");
        }
        strncpy(output_str + 2 + (i * 26), output_str_tmp, 26);
        light_mode_tmp = 0;
    }

    return OPRT_OK;
}


/**
 * @description: input_buf = onoff(1) + mode(1) + H(1) + V(1) + V(1) + B(2) + T(2) = 9
 * @param {type} none
 * @return: none
 */
OPERATE_RET opRealtimeDataHex2Str(UCHAR_T* input_buf, CHAR_T* output_str)
{
    UCHAR_T temp[22];
    UCHAR_T onoff = 0;
    UCHAR_T mode = 0;
    USHORT_T H = 0;
    USHORT_T S = 0;
    USHORT_T V = 0;
    USHORT_T B = 0;
    USHORT_T T = 0;
    onoff = input_buf[0];
    mode = input_buf[1];
    H = input_buf[2]<<8 + input_buf[3];
    S = input_buf[4]<<8 + input_buf[5];
    V = input_buf[6]<<8 + input_buf[7];
    B = input_buf[8]<<8 + input_buf[9];
    T = input_buf[10]<<8 + input_buf[11];

    memset(temp, 0, 22);
    snprintf((char *)temp, 22, "%02x%02x%04x%04x%04x%04x%04x", onoff, mode, H, S, V, B, T);
    strncpy(output_str, (const char *)temp, 21);


    return OPRT_OK;
}