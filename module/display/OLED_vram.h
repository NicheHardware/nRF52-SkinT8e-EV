
#ifndef _OLED_VRAM_H_
#define _OLED_VRAM_H_

#include "bitmap.h"

//#define OLED_WIDTH 64
//#define OLED_HEIGHT 32
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define MIN_SAME_SIZE 4


typedef enum {
	NONE,// 正常显示，不旋转
	CW_90D,// 顺时针90度
	CCW_90D,// 逆时针90度
	CW_180D,// 顺时针180度

} rotate_type_t;


void oled_Draw(sPOS* pos, const sBITMAP* bitmap, eBlendMode blendmode);
void oled_Update(void);
void oled_Update_without_clear(void);
sBITMAP* rotate_pic(rotate_type_t type, const sBITMAP* src);

extern void _oled_set_cursor(unsigned char x, unsigned char y);
extern void _oled_datas(unsigned char* data, unsigned char length);

#endif
