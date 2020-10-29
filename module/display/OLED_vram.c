
#include "OLED_vram.h"
#include "stdint.h"
#include "stdbool.h"

#ifndef MAX
	#define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif

#ifndef MIN
	#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef BITMAP
	#define BITMAP(x,y) bitmap->map[(x)+(y)*bitmap->w]
#endif

unsigned char oled_buffer[OLED_HEIGHT >> 3][OLED_WIDTH] = {0};
unsigned char oled_vram[OLED_HEIGHT >> 3][OLED_WIDTH] = {0};
unsigned char oled_screen[OLED_HEIGHT >> 3][OLED_WIDTH] = {0};


// draw to buffer
void oled_Draw(sPOS* pos, const sBITMAP* bitmap, eBlendMode blendmode)
{
	short notAlign = pos->y;
	//	unsigned char offset=pos->y/8;
	short h_t;	// 剩余高度
	unsigned char mask;
	unsigned char vy, by, yh;
	short i, j;
	//	short bline=0,sline=0;
	unsigned char bitmap_buffer[OLED_HEIGHT >> 3][OLED_WIDTH] = {0};
	sRECT vrect;	// 实际显示区域矩形
	sRECT brect = {0, 0, 0, 0};	// 位图显示区域矩形

	while(notAlign < 0) {
		notAlign += 256;
	}
	notAlign = (unsigned short)(notAlign) & 0x7;

	// 实际显示宽高
	vrect.x = MAX(0, pos->x);
	vrect.y = MAX(0, pos->y);
	vrect.w = MIN(OLED_WIDTH, pos->x + bitmap->w) - vrect.x;
	vrect.h = MIN(OLED_HEIGHT, pos->y + bitmap->h) - vrect.y;
	if(pos->x < 0) {
		brect.x = bitmap->w - vrect.w;
	}
	if(pos->y < 0) {
		brect.y = bitmap->h - vrect.h;
	}
	brect.w = vrect.w;
	brect.h = vrect.h;
	// 在画布外
	if(vrect.w <= 0 || vrect.h <= 0) {
		return;
	}


	vy = vrect.y;
	by = brect.y;

	while(vy < vrect.y + vrect.h) {
		if(vy & 0x7) {		// $by should be 0 here
			yh = (8 - vy & 0x7);
			for (i = 0; i < vrect.w; ++i) {
				bitmap_buffer[vy / 8][i + vrect.x] |= BITMAP(i + brect.x, by / 8) << (8 - yh);
			}
			vy += yh;
			by += yh;
		} else if(by & 0x7) {	//$vy&0x7 should be 0 here
			yh = (8 - by & 0x7);
			for (i = 0; i < vrect.w; ++i) {
				bitmap_buffer[vy / 8][i + vrect.x] |= BITMAP(i + brect.x, by / 8) >> (8 - yh);
			}
			vy += yh;
			by += yh;
		} else {
			yh = 8;
			for (i = 0; i < vrect.w; ++i) {
				bitmap_buffer[vy / 8][i + vrect.x] = BITMAP(i + brect.x, by / 8);
			}
			vy += 8;
			by += 8;
		}
	}

	h_t = vrect.h;
	notAlign = (unsigned short)(vrect.y) & 0x7;
	for (j = vrect.y / 8; ; j++) {
		if(notAlign && j == vrect.y / 8) {
			mask = 0xFF << notAlign;		// 底端对齐
			if(h_t >= 8 - notAlign) {
				h_t -= (8 - notAlign);
			} else {
				mask &= 0xFF >> (8 - notAlign - h_t);
				h_t = 0;
			}
		} else if(h_t < 8) {
			mask = 0xFF >> (8 - h_t);
			h_t = 0;
		} else {
			mask = 0xFF;
			h_t -= 8;
		}
		switch(blendmode) {
			default:
			case REPLACE:
				for (i = vrect.x; i < vrect.x + vrect.w; i++) {
					oled_buffer[j][i] = (oled_buffer[j][i] & ~mask) | (bitmap_buffer[j][i] & mask);
				}
				break;
			case OR:
				for (i = vrect.x; i < vrect.x + vrect.w; i++) {
					oled_buffer[j][i] |= bitmap_buffer[j][i] & mask;
				}
				break;
			case ERASE:
				for (i = vrect.x; i < vrect.x + vrect.w; i++) {
					oled_buffer[j][i] &= (~bitmap_buffer[j][i]);
				}
				break;
			case XOR:
				for (i = vrect.x; i < vrect.x + vrect.w; i++) {
					oled_buffer[j][i] ^= bitmap_buffer[j][i] & mask;
				}
				break;
			case AND:
				for (i = vrect.x; i < vrect.x + vrect.w; i++) {
					oled_buffer[j][i] &= (oled_buffer[j][i] & ~mask) | (bitmap_buffer[j][i] & mask);
				}
				break;
			case NOT:
				for (i = vrect.x; i < vrect.x + vrect.w; i++) {
					oled_buffer[j][i] |= (~bitmap_buffer[j][i])&mask;
				}
				break;
			case XNOR:
				for (i = vrect.x; i < vrect.x + vrect.w; i++) {
					oled_buffer[j][i] = ~(oled_buffer[j][i] ^ (bitmap_buffer[j][i] & mask));
				}
				break;
		}
		if(h_t == 0) {
			break;
		}
	}
}

// 高位是跳跃起点，低位是跳跃终点
unsigned int oled_vram_buffer_next_jump(unsigned char x, unsigned char y)
{
	unsigned int i;
	unsigned char same_count = 0;
	for(i = x; i < OLED_WIDTH; i++) {
		if(oled_vram[y][i] == oled_buffer[y][i]) {
			same_count += 1;
		} else {
			if(same_count > MIN_SAME_SIZE) {
				return ((i - same_count) << 8) | (i & 0xFF);
			}
			same_count = 0;
		}
	}
	if(same_count > MIN_SAME_SIZE) {
		return ((i - same_count) << 8) | (i & 0xFF);
	}
	return 0xFFFF;
}

//// update screen ram
//static void screen_update(void)
//{
//	unsigned int x, y;
//	for(y = 0; y<OLED_HEIGHT >> 3; y++)
//	{
//		for (x = 0; x < OLED_WIDTH; x++)
//		{
//			oled_screen[y][x] = oled_vram[y][x];
//		}
//	}
//}

// clear buffer
void buffer_clear(void)
{
	unsigned int x, y;
	for(y = 0; y<OLED_HEIGHT >> 3; y++) {
		for(x = 0; x < OLED_WIDTH; x++) {
			oled_buffer[y][x] = 0;
		}
	}
}

// clear vram
void vram_clear(void)
{
	unsigned int x, y;
	for(y = 0; y<OLED_HEIGHT >> 3; y++) {
		for (x = 0; x < OLED_WIDTH; x++) {
			oled_vram[y][x] = 0;
		}
	}
}

// update vram to OLED screen
void oled_Update(void)
{
	unsigned char i, x = 0, y = 0;
	unsigned char jump_start, jump_to;
	unsigned int jump;

	for(y = 0; y<OLED_HEIGHT >> 3; y++) {
		x = 0;
		while(x < OLED_WIDTH) {
			jump = oled_vram_buffer_next_jump(x, y);
			jump_start = (unsigned char)(jump >> 8);
			jump_to = (unsigned char)(jump & 0xFF);

			if(x != jump_start) {
				for (i = x; i < jump_start && i < OLED_WIDTH - 1; ++i) {
					oled_vram[y][i] = oled_buffer[y][i];
				}
				jump_start = jump_start > OLED_WIDTH ? OLED_WIDTH : jump_start;
				_oled_set_cursor(x, y);
				_oled_datas(&(oled_vram[y][x]), jump_start - x);
			}
			if(jump_to >= OLED_WIDTH) {
				break;
			}
			x = jump_to;
		}
	}
	buffer_clear();
}

// update vram to OLED screen
void oled_Update_without_clear(void)
{
	unsigned char i, x = 0, y = 0;
	unsigned char jump_start, jump_to;
	unsigned int jump;

	for(y = 0; y<OLED_HEIGHT >> 3; y++) {
		x = 0;
		while(x < OLED_WIDTH) {
			jump = oled_vram_buffer_next_jump(x, y);
			jump_start = (unsigned char)(jump >> 8);
			jump_to = (unsigned char)(jump & 0xFF);

			if(x != jump_start) {
				for (i = x; i < jump_start && i < OLED_WIDTH - 1; ++i) {
					oled_vram[y][i] = oled_buffer[y][i];
				}
				jump_start = jump_start > OLED_WIDTH ? OLED_WIDTH : jump_start;
				_oled_set_cursor(x, y);
				_oled_datas(&(oled_vram[y][x]), jump_start - x);
			}
			if(jump_to >= OLED_WIDTH) {
				break;
			}
			x = jump_to;
		}
	}
}


static uint8_t array_rotate[OLED_WIDTH * OLED_HEIGHT];
static sBITMAP map_rotate = {.w = 1, .h = 1, .map = array_rotate};

/**@brief 对像素点阵做旋转处理，本质上是将像素点从旧的坐标系中找出然后填入到新的坐标系中，关键点在于坐标系转换的关系，可图示简化分析
 *
 * @details
 *
 * @para [in]type 顺时针转90度，逆时针转90度，旋转180度
 *       [in]src 指向待处理像素点阵
 *
 * @return sBITMAP* 旋转过后的像素点阵
 */
sBITMAP* rotate_pic(rotate_type_t type, const sBITMAP* src)
{
	uint8_t selector, byte, i, j;
	int16_t r;
	uint16_t w_r = src->w; /* 剩余处理宽度 */
	uint16_t start_pos = src->w;
	uint16_t output_ptr = 0;

	int8_t column_step = 8;
	int8_t sigle_step_sign = 1;
	int8_t row_step = 0;
	bool byte_dir = true;

	bool MSB_flag = false;

	switch(type) {
		case CW_90D: {
			if(src->h > 8) {
				start_pos = (src->h / 8 - 1) * src->w;

				if(src->h % 8 != 0) {
					start_pos += src->w;
				}

			} else {
				start_pos = 0;
			}
			row_step = -src->w;

			column_step = 8;
			sigle_step_sign = 1;

			MSB_flag = true;
		}
		break;

		case CCW_90D: {
			start_pos = src->w - 1;
			row_step = src->w;

			column_step = -8;
			sigle_step_sign = -1;

			MSB_flag = false;

		}
		break;

		default: {
			return (sBITMAP*)src;
		}
	}
#if 0
	NRF_LOG_RAW_INFO("type:%d\r\n", type);
	NRF_LOG_RAW_INFO("start_pos:%d\r\n", start_pos);
	NRF_LOG_RAW_INFO("row_step:%d\r\n", row_step);
	NRF_LOG_RAW_INFO("column_step:%d\r\n", column_step);
	NRF_LOG_RAW_INFO("sigle_step_sign:%d\r\n", sigle_step_sign);

	NRF_LOG_RAW_INFO("=====>>src_font.w:%d\r\n", src->w);
	NRF_LOG_RAW_INFO("=====>>src_font.h:%d\r\n", src->h);
#endif
	while(w_r > 0) {
		r = 0;
		// 换行时更新取位符
		if(MSB_flag) {
			if(src->h % 8 == 0) {
				selector = 1 << 7;
			} else {
				selector = 1 << ((src->h % 8) - 1);
			}
		} else {
			selector = 0x01;
		}
		static uint16_t cnt = 0;

		for(j = 0; j < src->h; j++) {
			byte = 0;
			if(w_r >= 8) {
				for(i = 0; i < 8; i++) {
					if(byte_dir) {
						byte >>= 1;
					} else {
						byte <<= 1;
					}

					if(src->map[r + start_pos + sigle_step_sign * i]&selector) {
						if(byte_dir) {
							byte |= 0x80;
						} else {
							byte |= 0x01;
						}
					}
#if 0
					NRF_LOG_RAW_INFO("r:%d\r\n", r);
					NRF_LOG_RAW_INFO("start_pos:%d\r\n", start_pos);
					NRF_LOG_RAW_INFO("i:%d\r\n", i);
					NRF_LOG_RAW_INFO("r+start_pos+sigle_step_sign*i:%d\r\n", r + start_pos + sigle_step_sign * i);
					NRF_LOG_RAW_INFO("normal==[%d]selector:0x%02X--result:%d\r\n", cnt++, selector, (src->map[r + start_pos + sigle_step_sign * i]&selector));
#endif
				}

				if(MSB_flag) {
					selector >>= 1;
				} else {
					selector <<= 1;
				}

				array_rotate[output_ptr++] = byte;
				if(selector == 0) {
					r += row_step;

					if(MSB_flag) {
						selector = 0x80;
					} else {
						selector = 0x01;
					}
				}
			} else {
				for(i = 0; i < w_r; i++) {
					/* 当原图宽度不足8时处理 */
					if(byte_dir) {
						byte >>= 1;
					} else {
						byte <<= 1;
					}

					if(src->map[r + start_pos + sigle_step_sign * i]&selector) {
						if(byte_dir) {
							byte |= 0x80;
						} else {
							byte |= 0x01;
						}
					}
#if 0
					NRF_LOG_RAW_INFO("r:%d\r\n", r);
					NRF_LOG_RAW_INFO("start_pos:%d\r\n", start_pos);
					NRF_LOG_RAW_INFO("i:%d\r\n", i);
					NRF_LOG_RAW_INFO("r+start_pos+sigle_step_sign*i:%d\r\n", r + start_pos + sigle_step_sign * i);
					NRF_LOG_RAW_INFO("tail==[%d]selector:0x%02X--result:%d\r\n", cnt++, selector, (src->map[r + start_pos + sigle_step_sign * i]&selector));
#endif
				}
				if(MSB_flag) {
					selector >>= 1;
				} else {
					selector <<= 1;
				}

				if(byte_dir) {
					byte >>= (8 - w_r);
				} else {
					byte <<= (8 - w_r);
				}

				array_rotate[output_ptr++] = byte;
				if(selector == 0) {
					r += row_step;

					if(MSB_flag) {
						selector = 0x80;
					} else {
						selector = 0x01;
					}
				}
			}

		}

		(void)(cnt);

		if(w_r >= 8) {
			w_r -= 8;
			start_pos += column_step;
		} else {
			w_r = 0;
		}
	}

	map_rotate.w = src->h;
	map_rotate.h = src->w;
	map_rotate.map = array_rotate;

	return &map_rotate;

}



