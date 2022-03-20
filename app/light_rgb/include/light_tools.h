/*
 * @Author: jin lu
 * @email: jinlu@tuya.com
 * @Date: 2019-05-23 11:24:33
 * @file name:
 * @Description:
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 */

#ifndef __LIHGT_TOOLS_H__
#define __LIHGT_TOOLS_H__


#include "tuya_zigbee_sdk.h"
#include "light_types.h"
#include "app_common.h"


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


#define size_get(x) (sizeof(x)/sizeof(x[0]))

#define APP_COLOR_H_MAX                         360
#define APP_COLOR_S_MAX                         1000
#define APP_COLOR_V_MAX                         1000
#define APP_WHITE_B_MAX                         1000
#define APP_WHITE_T_MAX                         1000

#define ZIGBEE_COLOR_H_MAX                      254     //color: Hue
#define ZIGBEE_COLOR_S_MAX                      254     //color: Saturation
#define ZIGBEE_COLOR_V_MAX                      255     //color: value
#define ZIGBEE_WHITE_B_MAX                      255     //color: level
#define ZIGBEE_WHITE_T_MAX                      255     //color: temperature


USHORT_T zig_b_2_app_B(UCHAR_T zig_b);
UCHAR_T app_B_2_zig_b(USHORT_T app_B);
USHORT_T zig_h_2_app_H(UCHAR_T zig_h);
UCHAR_T app_H_2_zig_h(USHORT_T app_H);
USHORT_T zig_s_2_app_S(UCHAR_T zig_s);
UCHAR_T app_S_2_zig_s(USHORT_T app_S);


/**
 * @description: compare two string
 * @param {IN CHAR_T* str1 -> string 1}
 * @param {IN CHAR_T* str2 -> string 2}
 * @return: 0: if these two string is not same,
 *          1: these two string is all same
 */
BOOL_T b_string_compare(CHAR_T* str1, CHAR_T* str2);

/**
 * @berief: get the absolute value
 * @param { INT_T value -> calc value }
 * @return: absolute value
 * @retval: none
 */
UINT_T light_tool_get_abs_value(INT_T value);

/**
 * @berief: get the max value in 3 numbers
 * @param { UINT_T a -> number1 }
 * @param { UINT_T b -> number2 }
 * @param { UINT_T c -> number3 }
 * @return: Max value
 * @retval: none
 */
UINT_T light_tool_get_max_value(UINT_T a, UINT_T b, UINT_T c);

/**
 * @berief: ASSIC change to hex
 * @param { CHAR_T AscCode -> ASSIC code }
 * @return: hex value
 * @retval: none
 */
UCHAR_T light_tool_ASC_2_HEX(CHAR_T AscCode);

/**
 * @berief: four unsigned char merge into unsigned short
 * @param { UCHAR_T HH -> USHORT Hight hight 4bit }
 * @param { UCHAR_T HL -> USHORT Hight low 4bit   }
 * @param { UCHAR_T LH -> USHORT low hight 4bit   }
 * @param { UCHAR_T LL -> USHORT low low 4bit     }
 * @return: USHORT value
 * @retval: none
 */
USHORT_T light_tool_STR_2_USHORT(UCHAR_T HH, UCHAR_T HL, UCHAR_T LH, UCHAR_T LL);

/**
 * @berief: HSV change to RGB
 * @param {IN FLOAT_T h -> range 0~360 }
 * @param {IN FLOAT_T s -> range 0~1000}
 * @param {IN FLOAT_T v -> range 0~1000}
 * @param {OUT FLOAT_T *color_r -> R result,rang from 0~1000}
 * @param {OUT FLOAT_T *color_g -> G result,rang from 0~1000}
 * @param {OUT FLOAT_T *color_b -> B result,rang from 0~1000}
 * @return: none
 * @retval: none
 */
void light_tool_HSV_2_RGB(USHORT_T h, USHORT_T s, USHORT_T v, USHORT_T *r, USHORT_T *g, USHORT_T *b);

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
void light_tool_RGB_2_HSV(USHORT_T R, USHORT_T G, USHORT_T B, USHORT_T *H, USHORT_T *S, USHORT_T *V);

/**
 * @description:XY convert to RGB
 * @param {IN USHORT_T X} current X
 * @param {IN USHORT_T Y} current Y
 * @param {IN USHORT_T level} current level
 * @param {IN USHORT_T *r} out R
 * @param {IN USHORT_T *g} out G
 * @param {IN USHORT_T *b} out B
 * @return: none
 */
BOOL_T light_tool_XY_2_RGB(USHORT_T X, USHORT_T Y, USHORT_T level, USHORT_T *r, USHORT_T *g, USHORT_T *b);
/**
 * @description: RGB to XY
 * @param {r} 0~1000 range
 * @param {g} 0~1000 range
 * @param {b} 0~1000 range
 * @param {x} 0~1000 range
 * @param {y} 0~1000 range
 * @return: none
 */
void vLightToolRGB2XY(USHORT_T r, USHORT_T g, USHORT_T b, USHORT_T *x, USHORT_T *y);

/**
 * @description: HSV to XY
 * @param {h} 0~360 range
 * @param {S} 0~1000 range
 * @param {V} 0~1000 range
 * @param {x} CIE x
 * @param {y} CIE y
 * @return {type} none
 */
void vLightToolHSV2XY(USHORT_T H, USHORT_T S, USHORT_T V, USHORT_T *X, USHORT_T *Y);

/**
 * @description: XY to HSV
 *  @param {x} CIE x
 * @param {y} CIE y
 * @param {h} 0~360 range
 * @param {S} 0~1000 range
 * @param {V} 0~1000 range
 * @return {type} none
 */
void vLightToolXY2HSV(USHORT_T X, USHORT_T Y, USHORT_T Level, USHORT_T *H, USHORT_T *S, USHORT_T *V);


/**
 * @description: compress scene data,from string (max 2+26*8=210 bytes) to hex (8+8+64*8)bits = 66 bytes).
 * compressed data format:
 * scene id + mode + uint*8
 * @param {input_str} scene data string format
 * @param {output_buf} Output buffer address, compressed data.
 * @param {output_buf_len} output compressed data length
 * @return: 1:success / 0:failed
 */
OPERATE_RET op_scene_data_compress(CHAR_T* input_str, UCHAR_T* output_buf, UINT_T *output_buf_len);

/**
 * @description: decompress scene data to string format
 * @param {input_buf} compressed scene data
 * @param {input_buf_len} compressed scene data length
 * @param {output_str} decompressed scene data,string format
 * @return: OPERATE_RET
 */
OPERATE_RET opSceneDataDecompress(UCHAR_T* input_buf, UINT_T input_buf_len, CHAR_T* output_str);


/**
 * @description: decompress one uint scene data, to string format
 * @param {input_buf} scene compressed data
 * @param {light_mode} mode value
 * @param {output_str} output compressed data length
 * @return: OPERATE_RET
 */
extern OPERATE_RET opDecompressSceneDataOneUint(UCHAR_T *input_buf, UCHAR_T light_mode, CHAR_T * output_str);

/**
 * @description: input_buf = onoff(1) + mode(1) + H(1) + V(1) + V(1) + B(2) + T(2) = 9
 * @param {type} none
 * @return: none
 */
OPERATE_RET opRealtimeDataHex2Str(UCHAR_T* input_buf, CHAR_T* output_str);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __LIHGT_TOOLS_H__ */
