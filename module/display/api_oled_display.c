
#include "app_timer.h"
#include "font_source.h"
#include "oled_vram.h"
#include "api_oled_display.h"

#ifdef USE_EXTERNAL_FONT
	#include "font_extract_driver.h"
#endif//USE_EXTERNAL_FONT

#include "hal_oled_ssd1306_128x64.h"

void display_init(void)
{
	oled_init();
}

/**@brief 输入起点坐标和像素点阵，显示单个元素
 *
 * @para
 * [in] pos_h 逻辑上的横坐标
 * [in] pos_v 逻辑上的纵坐标
 * [in] font_src 包含尺寸的像素点点阵
 *
 * @note 左上角为逻辑起始点
 */
void display_single_element(int16_t pos_h, int16_t pos_v, const sBITMAP* src, rotate_type_t type)
{
	// 输入参数有误
	if(src == NULL || pos_h > POS_H_MAX || pos_v > POS_L_MAX ) {
		return;
	}

	const sBITMAP* font_src = src;

	sPOS log_pos1;// 起始点逻辑坐标
	sPOS pah_pos2;// 起始点物理坐标
	sPOS w_pos;// 实际操作坐标

	log_pos1.x = pos_h;
	log_pos1.y = pos_v;


	switch(type) {
		case CW_90D: {
			font_src = rotate_pic(CW_90D, src);

			pah_pos2.x = OLED_DRAW_WIDTH - log_pos1.y;
			pah_pos2.y = log_pos1.x;

			// 变换坐标系
			w_pos.x = pah_pos2.x - font_src->w;
			w_pos.y = pah_pos2.y;
		}
		break;

		case CCW_90D: {
			font_src = rotate_pic(CCW_90D, src);

			// 变换坐标系
			pah_pos2.x = log_pos1.y;
			pah_pos2.y = OLED_DRAW_HEIGHT - log_pos1.x;

			w_pos.x = pah_pos2.x;
			w_pos.y = pah_pos2.y - font_src->h;

		}
		break;

		default: {
			font_src = src;

			pah_pos2.x = log_pos1.x;
			pah_pos2.y = log_pos1.y;

			w_pos.x = pah_pos2.x;
			w_pos.y = pah_pos2.y;
		}
		break;

	}

#if 1    // 屏幕本身旋转了，则使用原有坐标
	pah_pos2.x = log_pos1.x;
	pah_pos2.y = log_pos1.y;

	w_pos.x = pah_pos2.x;
	w_pos.y = pah_pos2.y;
#endif
	oled_Draw(&w_pos, font_src, REPLACE);
}

/**@brief 获取一个正整数的某个数位上的值，比如320的个位是0 即func(1,320)=0
 *
 * @param [in] value  输入的正整数
 *
 *        [in] index  要获取的数位，1代表个位，不得超过第七位
 *
 * @return 对应数位上的值，当输入参数不支持时返回0
 */
uint8_t get_index_value_of_value(uint8_t index, uint32_t value)
{
	uint8_t result = 0;
	switch(index) {
		case 1: {
			result = value % 10;
		}
		break;

		case 2: {
			result = (value / 10) % 10;
		}
		break;

		case 3: {
			result = (value / 100) % 10;
		}
		break;

		case 4: {
			result = (value / 1000) % 10;
		}
		break;
		case 5: {
			result = (value / 10000) % 10;
		}
		break;
		case 6: {
			result = (value / 100000) % 10;
		}
		break;
		case 7: {
			result = (value / 100000) % 10;
		}
		break;
		default:
			break;

	}

	return result;
}


/**@brief 显示单个数字或符号
 *
 */
void display_single_digit(int16_t pos_h, int16_t pos_v, digit_size_type_t type, uint8_t c_value)
{
	if(pos_h >= POS_H_MAX || pos_v >= POS_L_MAX) {
		SXP_LOG_OLED("display_single_character_error:unvalid_para\r\n");
	}

	const sBITMAP* data_src = NULL;
	int16_t cur_h, cur_v;

	cur_h = pos_h;
	cur_v = pos_v;

	switch(type) {
		case DIGIT_SIZE_10X23_1: {
			data_src = num_10x23_1_array;
			c_value %= 10;
		}
		break;

		case DIGIT_SIZE_11X23_2: {
			data_src = num_11x23_2_array;
			c_value %= 10;
		}
		break;

		case DIGIT_SIZE_10X23_3: {
			data_src = num_10x23_3_array;
			c_value %= 10;
		}
		break;

		case DIGIT_SIZE_8X19_1: {
			data_src = num_8x19_1_array;
			c_value %= 10;
		}
		break;

		case DIGIT_SIZE_9X19_2: {
			data_src = num_9x19_2_array;
			c_value %= 10;
		}
		break;

		case DIGIT_SIZE_8X19_3: {
			data_src = num_8x19_3_array;
			c_value %= 10;
		}
		break;

		case DIGIT_SIZE_4X9: {
			data_src = num_4x9_array;
			c_value %= 10;
		}
		break;

		case DIGIT_SIZE_BAR_18X19: {
			data_src = bar_18x19_array;
			c_value %= 1;
		}
		break;

		case DIGIT_SIZE_COLON_2X23_1: {
			data_src = colon_2x23_1_array;
			c_value %= 1;
		}
		break;

		case DIGIT_SIZE_COLON_1X23_2: {
			data_src = colon_1x23_2_array;
			c_value %= 1;
		}
		break;

		case DIGIT_SIZE_COLON_1X23_3: {
			data_src = colon_1x23_3_array;
			c_value %= 10;
		}
		break;

		default:
			break;
	}

	if(data_src == NULL) {
		return;
	}

	display_single_element(cur_h, cur_v, &data_src[c_value], NONE);

}


/**@brief 显示数字,1位或多位
 *
 * 输入起始位置，自动居中对齐
 *
 */
void display_digit_value(int16_t start_pos_h, int16_t start_pos_l, int16_t end_pos_h, uint8_t interval, digit_size_type_t digit_type, uint32_t in_value)
{
	int16_t cur_h, cur_v;
	uint8_t disp_num = 1; // 位数
	uint32_t t_value = in_value;

	const sBITMAP* data_src = NULL;

	// 得出数字是几位数
	for(uint8_t i = 0; i < 10; i++) {
		t_value /= 10;
		if(t_value) {
			disp_num++;
		} else {
			break;
		}
	}

	switch(digit_type) {
		case DIGIT_SIZE_10X23_1: {
			data_src = num_10x23_1_array;
		}
		break;

		case DIGIT_SIZE_11X23_2: {
			data_src = num_11x23_2_array;
		}
		break;

		case DIGIT_SIZE_10X23_3: {
			data_src = num_10x23_3_array;
		}
		break;

		case DIGIT_SIZE_8X19_1: {
			data_src = num_8x19_1_array;
		}
		break;

		case DIGIT_SIZE_9X19_2: {
			data_src = num_9x19_2_array;
		}
		break;

		case DIGIT_SIZE_8X19_3: {
			data_src = num_8x19_3_array;
		}
		break;

		case DIGIT_SIZE_4X9: {
			data_src = num_4x9_array;
		}
		break;
		default:
			break;
	}


	if(data_src == NULL) {
		return;
	}

	// 计算起始地址
	cur_h = start_pos_h + ((end_pos_h - start_pos_h) - (disp_num * data_src->w + (disp_num - 1) * interval)) / 2;
	cur_v = start_pos_l;

	for(; disp_num > 0; disp_num--) {
		display_single_element(cur_h, cur_v, &data_src[get_index_value_of_value(disp_num, in_value)], NONE);

		cur_h += data_src->w + interval; // 更新位置
	}

}

#ifdef USE_EXTERNAL_FONT

/**@brief 从 字库芯片中提取出一个字符并显示在OLED上
 *
 * input: pos_h,横坐标
 *       pos_v，纵坐标
 *       flash_font_type,字符类型
 *       UNICODE,字符对应的UNICODE
 *
 * example:
 * display_flash_font_character(0,10,FLASH_FONT_ASCII_8X16,'m');
 * display_flash_font_character(10,10,FLASH_FONT_CHINESE,0x6211);
 * display_flash_font_character(10+16,10,FLASH_FONT_JAPANESE,0x611B);
 * display_flash_font_character(10+32,10,FLASH_FONT_KOREAN,0xB108);
 *
 */
void display_flash_font_character(int16_t pos_h, int16_t pos_v, flash_font_type_t flash_font_type, uint32_t UNICODE)
{
	sBITMAP p_font, *p_after_modify = NULL;

	uint8_t dataBuf[64] = {0};
	p_font.map = dataBuf;

	switch (flash_font_type) {
		case FLASH_FONT_ASCII_5X7: {
			p_font.w = 5;
			p_font.h = 7;
		}
		break;
		case FLASH_FONT_ASCII_7X8: {
			p_font.w = 7;
			p_font.h = 8;
		}
		break;
		case FLASH_FONT_ASCII_6X12: {
			p_font.w = 6;
			p_font.h = 12;
		}
		break;
		case FLASH_FONT_ASCII_12_B_A:
		case FLASH_FONT_ASCII_12_B_T: {
			p_font.w = 12;
			p_font.h = 12;
		}
		break;
		case FLASH_FONT_ASCII_8X16: {
			p_font.w = 8;
			p_font.h = 16;
		}
		break;
		case FLASH_FONT_ASCII_16_A:
		case FLASH_FONT_ASCII_16_T: {
			p_font.w = 16;
			p_font.h = 16;
		}
		break;
		case FLASH_FONT_ASCII_12X24: {
			p_font.w = 12;
			p_font.h = 24;
		}
		break;
		case FLASH_FONT_ASCII_24_B: {
			p_font.w = 24;
			p_font.h = 24;
		}
		break;
		case FLASH_FONT_ASCII_16X32: {
			p_font.w = 16;
			p_font.h = 32;
		}
		break;

		case FLASH_FONT_ASCII_32_B: {
			p_font.w = 32;
			p_font.h = 32;
		}
		break;

		case FLASH_FONT_CHINESE:
		case FLASH_FONT_JAPANESE:
		case FLASH_FONT_KOREAN: {
			p_font.w = 16;
			p_font.h = 16;
		}
		break;

		default:
			break;

	}

	if(flash_font_type >= FLASH_FONT_ASCII_START && flash_font_type <= FLASH_FONT_ASCII_END) {
		get_ascii_font_from_flash(dataBuf, flash_font_type, UNICODE);

	} else if(flash_font_type == FLASH_FONT_CHINESE) {
		get_chinese_font_from_flash(dataBuf, UNICODE);
	} else if(flash_font_type == FLASH_FONT_JAPANESE) {
		get_japanese_font_from_flash(dataBuf, UNICODE);
	} else if(flash_font_type == FLASH_FONT_KOREAN) {
		get_korean_font_from_flash(dataBuf, UNICODE);
	}

#if 0
	p_after_modify = rotate_pic(CCW_90D, &p_font);

	display_single_element(pos_h, pos_v, p_after_modify);

#else
	UNUSED_VARIABLE(p_after_modify);
	display_single_element(pos_h, pos_v, &p_font, NONE);

	SXP_LOG_OLED("WARNING:p_rotate_handler_is_NULL,current_font_type:%d\r\n", flash_font_type);
#endif

}

/*

*/


/**@brief 计算出utf8的字节数
 *
 *     utf8 编码规则
 *
 *     1) 对于单字节的符号, 字节的第一位设为0, 后面7位为这个符号的unicode码. 因此对于
 *        英语字母, UTF-8编码和ASCII码是同样的.
 *
 *     2) 对于n字节的符号(n>1), 第一个字节的前n位都设为1, 第n+1位设为0, 后面字节的前
 *        两位一律设为10. 剩下的没有提及的二进制位, 所有为这个符号的unicode码.
 *
 *   |  Unicode符号范围      |  UTF-8编码方式
 * n |  (十六进制)           | (二进制)
 * --+-----------------------+------------------------------------------------------
 * 1 | 0000 0000 - 0000 007F |                                              0xxxxxxx
 * 2 | 0000 0080 - 0000 07FF |                                     110xxxxx 10xxxxxx
 * 3 | 0000 0800 - 0000 FFFF |                            1110xxxx 10xxxxxx 10xxxxxx
 * 4 | 0001 0000 - 0010 FFFF |                   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 5 | 0020 0000 - 03FF FFFF |          111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 6 | 0400 0000 - 7FFF FFFF | 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 */
static uint8_t enc_get_utf8_size(uint8_t data)
{
	uint8_t num = 0;

	for(uint8_t i = 0; i < 8; i++) {
		// 计算1的个数 遇到 0 则结束
		if((data & GET_BIT(7 - i)) == 0) {
			break;
		}

		num++;
	}

	return num;

}

/**@brief 将一个字符的Unicode(UCS-2和UCS-4)编码转换成UTF-8编码.
 *
 * 參数:
 *    unic     字符的Unicode编码值
 *    pOutput  指向输出的用于存储UTF8编码值的缓冲区的指针
 *    outsize  pOutput缓冲的大小
 *
 * 返回值:
 *    返回转换后的字符的UTF8编码所占的字节数, 假设出错则返回 0 .
 *
 * 注意:
 *     1. UTF8没有字节序问题, 可是Unicode有字节序要求;
 *        字节序分为大端(Big Endian)和小端(Little Endian)两种;
 *        在Intel处理器中採用小端法表示, 在此採用小端法表示. (低地址存低位)
 *     2. 请保证 pOutput 缓冲区有最少有 6 字节的空间大小!
 */
//static int enc_unicode_to_utf8_one(unsigned long unic, unsigned char *pOutput,
//        int outSize)
//{
//    ASSERT(pOutput != NULL);
//    ASSERT(outSize >= 6);
//
//    if ( unic <= 0x0000007F )
//    {
//        // * U-00000000 - U-0000007F:  0xxxxxxx
//        *pOutput     = (unic & 0x7F);
//        return 1;
//    }
//    else if ( unic >= 0x00000080 && unic <= 0x000007FF )
//    {
//        // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
//        *(pOutput+1) = (unic & 0x3F) | 0x80;
//        *pOutput     = ((unic >> 6) & 0x1F) | 0xC0;
//        return 2;
//    }
//    else if ( unic >= 0x00000800 && unic <= 0x0000FFFF )
//    {
//        // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
//        *(pOutput+2) = (unic & 0x3F) | 0x80;
//        *(pOutput+1) = ((unic >>  6) & 0x3F) | 0x80;
//        *pOutput     = ((unic >> 12) & 0x0F) | 0xE0;
//        return 3;
//    }
//    else if ( unic >= 0x00010000 && unic <= 0x001FFFFF )
//    {
//        // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
//        *(pOutput+3) = (unic & 0x3F) | 0x80;
//        *(pOutput+2) = ((unic >>  6) & 0x3F) | 0x80;
//        *(pOutput+1) = ((unic >> 12) & 0x3F) | 0x80;
//        *pOutput     = ((unic >> 18) & 0x07) | 0xF0;
//        return 4;
//    }
//    else if ( unic >= 0x00200000 && unic <= 0x03FFFFFF )
//    {
//        // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
//        *(pOutput+4) = (unic & 0x3F) | 0x80;
//        *(pOutput+3) = ((unic >>  6) & 0x3F) | 0x80;
//        *(pOutput+2) = ((unic >> 12) & 0x3F) | 0x80;
//        *(pOutput+1) = ((unic >> 18) & 0x3F) | 0x80;
//        *pOutput     = ((unic >> 24) & 0x03) | 0xF8;
//        return 5;
//    }
//    else if ( unic >= 0x04000000 && unic <= 0x7FFFFFFF )
//    {
//        // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
//        *(pOutput+5) = (unic & 0x3F) | 0x80;
//        *(pOutput+4) = ((unic >>  6) & 0x3F) | 0x80;
//        *(pOutput+3) = ((unic >> 12) & 0x3F) | 0x80;
//        *(pOutput+2) = ((unic >> 18) & 0x3F) | 0x80;
//        *(pOutput+1) = ((unic >> 24) & 0x3F) | 0x80;
//        *pOutput     = ((unic >> 30) & 0x01) | 0xFC;
//        return 6;
//    }
//
//    return 0;
//}


/**@brief
 * 将一个字符的UTF8编码转换成Unicode(UCS-2和UCS-4)编码.
 *
 * 參数:
 *    pInput      指向输入缓冲区, 以UTF-8编码
 *    Unic        指向输出缓冲区, 其保存的数据即是Unicode编码值,
 *                类型为unsigned long .
 *
 * 返回值:
 *    成功则返回该字符的UTF8编码所占用的字节数; 失败则返回0.
 *
 * 注意:
 *     1. UTF8没有字节序问题, 可是Unicode有字节序要求;
 *        字节序分为大端(Big Endian)和小端(Little Endian)两种;
 *        在Intel处理器中採用小端法表示, 在此採用小端法表示. (低地址存低位)
 */
static int enc_utf8_to_unicode_one(const unsigned char* pInput, unsigned long* Unic)
{
	ASSERT(pInput != NULL && Unic != NULL);

	// b1 表示UTF-8编码的pInput中的高字节, b2 表示次高字节, ...
	char b1, b2, b3, b4, b5, b6;

	*Unic = 0x0; // 把 *Unic 初始化为全零
	int utfbytes = enc_get_utf8_size(*pInput);
	unsigned char* pOutput = (unsigned char*) Unic;

	switch ( utfbytes ) {
		case 0:
			*pOutput     = *pInput;
			utfbytes    += 1;
			break;
		case 2:
			b1 = *pInput;
			b2 = *(pInput + 1);
			if ( (b2 & 0xE0) != 0x80 ) {
				return 0;
			}
			*pOutput     = (b1 << 6) + (b2 & 0x3F);
			*(pOutput + 1) = (b1 >> 2) & 0x07;
			break;
		case 3:
			b1 = *pInput;
			b2 = *(pInput + 1);
			b3 = *(pInput + 2);
			if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) ) {
				return 0;
			}
			*pOutput     = (b2 << 6) + (b3 & 0x3F);
			*(pOutput + 1) = (b1 << 4) + ((b2 >> 2) & 0x0F);
			break;
		case 4:
			b1 = *pInput;
			b2 = *(pInput + 1);
			b3 = *(pInput + 2);
			b4 = *(pInput + 3);
			if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
			        || ((b4 & 0xC0) != 0x80) ) {
				return 0;
			}
			*pOutput     = (b3 << 6) + (b4 & 0x3F);
			*(pOutput + 1) = (b2 << 4) + ((b3 >> 2) & 0x0F);
			*(pOutput + 2) = ((b1 << 2) & 0x1C)  + ((b2 >> 4) & 0x03);
			break;
		case 5:
			b1 = *pInput;
			b2 = *(pInput + 1);
			b3 = *(pInput + 2);
			b4 = *(pInput + 3);
			b5 = *(pInput + 4);
			if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
			        || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80) ) {
				return 0;
			}
			*pOutput     = (b4 << 6) + (b5 & 0x3F);
			*(pOutput + 1) = (b3 << 4) + ((b4 >> 2) & 0x0F);
			*(pOutput + 2) = (b2 << 2) + ((b3 >> 4) & 0x03);
			*(pOutput + 3) = (b1 << 6);
			break;
		case 6:
			b1 = *pInput;
			b2 = *(pInput + 1);
			b3 = *(pInput + 2);
			b4 = *(pInput + 3);
			b5 = *(pInput + 4);
			b6 = *(pInput + 5);
			if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
			        || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80)
			        || ((b6 & 0xC0) != 0x80) ) {
				return 0;
			}
			*pOutput     = (b5 << 6) + (b6 & 0x3F);
			*(pOutput + 1) = (b5 << 4) + ((b6 >> 2) & 0x0F);
			*(pOutput + 2) = (b3 << 2) + ((b4 >> 4) & 0x03);
			*(pOutput + 3) = ((b1 << 6) & 0x40) + (b2 & 0x3F);
			break;
		default:
			return 0;
	}

	return utfbytes;
}


void convert_ascii_to_unicode(uint32_t* unicode_buf, uint8_t* ascii_buf, uint16_t len)
{
	for(uint16_t i = 0; i < len; i++) {
		unicode_buf[i] = ascii_buf[i];
		//		SXP_LOG_OLED("origin[%d]:%4x, converted[%d]:%4x\r\n",i,ascii_buf[i],i,unicode_buf[i]);
	}
}

/**@brief utf8转换成unicode
 *
 * @param [in] utf8_buf 指向被转换utf8数据的指针
 *
 *        [in] utf8_buflen 被转换数据的个数
 *
 *        [out] unicode_buf 转换出来的unicode存放位置
 *
 *        [out] utf8_buflen 指针,转换出来的unicode个数
 *
 */
void convert_utf8_to_unicode(uint32_t* unicode_buf, uint16_t* unicode_bufLen, uint8_t* utf8_buf, uint16_t utf8_bufLen)
{
	uint8_t utf8_size = 0;

	uint16_t utf8_head_index = 0, unicode_index = 0;

	//
	for( uint16_t i = 0; i < utf8_bufLen ;) {
		// ascii
		if( utf8_buf[i] <= 0x7F ) {

			unicode_buf[unicode_index] = utf8_buf[i];

			unicode_index++;
			utf8_head_index++;

			i++;
		}
		// 其他字符
		else {
			utf8_size = enc_get_utf8_size( utf8_buf[utf8_head_index] );

			enc_utf8_to_unicode_one(&utf8_buf[utf8_head_index], (unsigned long*)&unicode_buf[unicode_index]);

			utf8_head_index += utf8_size;

			unicode_index++;

			i += utf8_size;
		}

		// 达到上限，退出循环
		if(unicode_index >= MAX_UNICODE_BUF_LEN) {
			break;
		}

	}

	*unicode_bufLen = unicode_index;

}

/**@brief 智能识别语言
 *
 */
flash_font_type_t recognize_language(uint32_t unicode)
{
	flash_font_type_t font_type;

	// ASCII
	if( unicode <= 0x7F ) {
		font_type = FLASH_FONT_ASCII_8X16;
	}
	// 中文
	else if( unicode >= 0x4E00 && unicode <= 0x9FA5 ) {
		font_type = FLASH_FONT_CHINESE;
	}
	// 日语
	else if( unicode >= 0x800 && unicode <= 0x4E00 ) {
		font_type = FLASH_FONT_JAPANESE;
	}
	// 韩语
	else if( unicode >= 0xAC00 && unicode <= 0xD7A3 ) {
		font_type = FLASH_FONT_KOREAN;
	}
	//   未识别出
	else {
		font_type = FLASH_FONT_UNKNOW;
	}

	return font_type;

}


/**@brief 显示unicode字符串，支持中日韩语言，asciii各种尺寸字体
 *
 * @details 会调用到flash中的字库
 *
 */
void display_flash_font_string(int16_t start_pos_h, int16_t start_pos_l, uint8_t interval, flash_font_type_t flash_font_type, uint32_t* UNICODE_BUF, uint8_t len)
{
	int16_t w_pos_h, w_pos_l;

	w_pos_h = start_pos_h;
	w_pos_l = start_pos_l;

	uint8_t w_index = 0;

	uint8_t size_h = 0;

	if(flash_font_type == FLASH_FONT_UNKNOW) {
		return;
	}

	switch(flash_font_type) {

		case FLASH_FONT_ASCII_5X7: {
			size_h = 5;
		}
		break;

		case FLASH_FONT_ASCII_7X8: {
			size_h = 7;
		}
		break;
		case FLASH_FONT_ASCII_6X12: {
			size_h = 6;
		}
		break;

		case FLASH_FONT_ASCII_8X16: {
			size_h = 8;
		}
		break;
		case FLASH_FONT_CHINESE:
		case FLASH_FONT_JAPANESE:
		case FLASH_FONT_KOREAN: {
			size_h = 16;
		}
		break;

		default:
			break;
	}

	while(w_index < len) {

		display_flash_font_character(w_pos_h, w_pos_l, flash_font_type, UNICODE_BUF[w_index]);

		w_index++;
		w_pos_h = w_pos_h + size_h + interval;

		// 在这里写换行处理
	}

}

/**@brief 显示一串UTF-8字符
 *
 * @details 会调用到flash中的字库
 *
 */
void display_utf8(int16_t start_pos_h, int16_t start_pos_l, uint8_t interval, uint8_t* utf8_buf, uint16_t utf8_bufLen)
{

	uint32_t unicode_buf[MAX_UNICODE_BUF_LEN];

	uint16_t dlen;

	flash_font_type_t type;

	convert_utf8_to_unicode(unicode_buf, &dlen, utf8_buf, utf8_bufLen);

	SXP_LOG_OLED("unicode_len_is:%d\r\n", dlen);

	uint8_t size_h = 0;
	int16_t w_pos_h = start_pos_h;

	for( uint8_t i = 0; i < dlen; i++ ) {
		type = recognize_language(unicode_buf[i]);

		SXP_LOG_OLED("recognize_language_is:%d\r\n", type);

		switch(type) {

			case FLASH_FONT_ASCII_5X7: {
				size_h = 5;
			}
			break;

			case FLASH_FONT_ASCII_7X8: {
				size_h = 7;
			}
			break;
			case FLASH_FONT_ASCII_6X12: {
				size_h = 6;
			}
			break;

			case FLASH_FONT_ASCII_8X16: {
				size_h = 8;
			}
			break;
			case FLASH_FONT_CHINESE:
			case FLASH_FONT_JAPANESE:
			case FLASH_FONT_KOREAN: {
				size_h = 16;
			}
			break;

			default: {
				size_h = 8;
			}
			break;

		}

		// 未识别的字符不显示，但是要留出空白
		if( type != FLASH_FONT_UNKNOW ) {
			display_flash_font_character(w_pos_h, start_pos_l, type, unicode_buf[i]);
		}

		w_pos_h += size_h + interval;

	}

}

/**@brief 计算utf字符串的的像素宽度
 *
 * @details
 *
 */
uint16_t compute_utf8_pixels_width(uint8_t* utf8_buf, uint16_t utf8_bufLen, uint8_t interval)
{
	//
	uint32_t unicode_buf[MAX_UNICODE_BUF_LEN];

	uint16_t dlen, pixels_width = 0;

	flash_font_type_t type;

	convert_utf8_to_unicode(unicode_buf, &dlen, utf8_buf, utf8_bufLen);

	SXP_LOG_OLED("unicode_len_is:%d\r\n", dlen);

	uint8_t size_h = 0;

	for( uint8_t i = 0; i < dlen; i++ ) {
		type = recognize_language(unicode_buf[i]);

		SXP_LOG_OLED("recognize_language_is:%d\r\n", type);

		switch(type) {

			case FLASH_FONT_ASCII_5X7: {
				size_h = 5;
			}
			break;

			case FLASH_FONT_ASCII_7X8: {
				size_h = 7;
			}
			break;
			case FLASH_FONT_ASCII_6X12: {
				size_h = 6;
			}
			break;

			case FLASH_FONT_ASCII_8X16: {
				size_h = 8;
			}
			break;
			case FLASH_FONT_CHINESE:
			case FLASH_FONT_JAPANESE:
			case FLASH_FONT_KOREAN: {
				size_h = 16;
			}
			break;

			default: {
				size_h = 8;
			}
			break;

		}

		pixels_width += size_h + interval;

	}

	return pixels_width;

}

#endif // USE_EXTERNAL_FONT

/**@brief 显示自制点阵字库ASCII
 *
 * @details
 *
 */
void display_ascii(int16_t pos_h, int16_t pos_v, uint8_t* disp_data_src, uint8_t len)
{
	if(len == 0 || disp_data_src == NULL) {
		SXP_LOG_OLED("display_ascii_error:unvalid_para\r\n");
	}

	const sBITMAP* data_src;
	int16_t cur_h, cur_v;
	uint8_t disp_index = 0;

	cur_h = pos_h;
	cur_v = pos_v;
	data_src = &ASCII8X16_array[0];

	for(; len > 0; len--) {
		//		SXP_LOG_OLED("display_element:%d\r\n",disp_index);

		display_single_element(cur_h, cur_v, &data_src[disp_data_src[disp_index] - 32], CW_90D);

		cur_h += data_src->h; // 更新位置

		disp_index++;
		// 可以在这里写换行

		if(cur_h >= OLED_DRAW_WIDTH) {
			cur_h = pos_h;

			cur_v = pos_v + 16;
		}
	}

}

void display_icon(int8_t start_x, int8_t start_y, uint8_t display_object, icon_type_t icon_type, uint8_t icon_index)
{
	if(display_object == 0) {
		return;
	}

	const sBITMAP* code_src;
	rotate_type_t rotate_type = NONE;

	switch(icon_type) {
		case ICON_TYPE_STEP_RUN: {
			code_src = &run_array[icon_index % 8];
		}
		break;

		case ICON_TYPE_STEP_RUN_FAST: {
			code_src = &run_fast_array[icon_index % 8];
		}
		break;

		case ICON_TYPE_STEP_WALK: {
			code_src = &walk_array[icon_index % 5];
		}
		break;

		case ICON_TYPE_HEART: {
			code_src = &heart_icon;
		}
		break;
		case ICON_TYPE_CAMERA: {
			code_src = &photo_array[icon_index % 3];
		}
		break;
		case ICON_TYPE_BT_ADV: {
			code_src = &BT_advertise_array[icon_index % 3];
		}
		break;
		case ICON_TYPE_BT_CONN: {
			code_src = &BT_connect_array[icon_index % 3];
		}
		break;
		case ICON_TYPE_BT_DIS_CONN: {
			code_src = &BT_disconnect_array[icon_index % 3];
		}
		break;
		case ICON_TYPE_MSG: {
			code_src = &msg_reminder_array[icon_index % 3];
		}
		break;
		case ICON_TYPE_CALL: {
			code_src = &call_reminder_array[icon_index % 3];
		}
		break;
		case ICON_TYPE_ALARM: {
			code_src = &alarm_reminder_array[icon_index % 2];
		}
		break;

		case ICON_TYPE_LOW_POWER_40: {
			code_src = &low_power_40_array[icon_index % 3];
		}
		break;

		case ICON_TYPE_LOW_POWER_20: {
			code_src = &low_power_20_array[icon_index % 3];
		}
		break;
		case ICON_TYPE_LACK_POWER: {
			code_src = &lack_power_array[icon_index % 2];
		}
		break;
		case ICON_TYPE_CHARGING: {
			code_src = &charging_array[icon_index % 6];
		}
		break;
		case ICON_TYPE_BT_STATE_CONN: {
			code_src = &bt_state_conn_icon;
		}
		break;
		case ICON_TYPE_BT_STATE_DIS_CONN: {
			code_src = &bt_state_disconn_icon;
		}
		break;
		case ICON_TYPE_HEART_UNIT: {
			code_src = &bpm_icon;
		}
		break;
		case ICON_TYPE_WEEK_NO: {
			code_src = &week_no_icon;
		}
		break;
		case ICON_TYPE_WEEK_YES: {
			code_src = &week_yes_icon;
		}
		break;

		case ICON_TYPE_HEART_RUNNING_ANI: {
			code_src = &heart_detecting_icon_array[icon_index % 3];
		}
		break;

		case ICON_TYPE_HEART_DETECTING_ANI: {
			code_src = &heart_detecting_waveform_array[icon_index % 4];
		}
		break;

		case ICON_TYPE_BLANK: {
			code_src = &blank_16x16_icon;
		}
		break;

		case ICON_TYPE_OTA: {
			code_src = &OTA_array[icon_index % 3];
		}
		break;
		case ICON_TYPE_MANUFACTURE: {
			code_src = &corumi_icon;
		}
		break;
		default: {
			SXP_LOG_OLED("display_icon_error_unvalid_type:%d\r\n", type);
		}

	}

	display_single_element(start_x, start_y, code_src, rotate_type);


}

const unsigned char baidu[336] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xF8, 0xF8, 0xFC, 0xFC, 0xFC, 0xF8, 0xF0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x80, 0xE0, 0xF0, 0xF0, 0xF8, 0xF8, 0xF0, 0xF0, 0xE0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x1F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xF8, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x03, 0x03, 0x01, 0x81, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFF, 0xFF, 0x0F, 0x0F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF8, 0xF0, 0xE0, 0xC0, 0x83, 0x0F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0F, 0x07, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFC, 0xFE, 0xFF, 0xFF, 0x07, 0x03, 0x01, 0x60, 0xF0, 0xF8, 0xF8, 0xF8, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFE, 0xFE, 0xFC, 0xF8, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x1F, 0x3F, 0x7F, 0x7F, 0x7E, 0x78, 0x78, 0x70, 0x71, 0x71, 0x71, 0x71, 0x70, 0x70, 0x30, 0x30, 0x3F, 0x38, 0x38, 0x30, 0x70, 0x71, 0x71, 0x71, 0xF0, 0xF0, 0xF0, 0xF0, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x1F, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
const sBITMAP baidu_icon = {48, 51, baidu};


#include "nrf_delay.h"

/**@brief 测试SPI刷屏OLED_12864，实际SPI速率可达6.8MHZ,刷屏一帧的时长为1.5ms(12864全屏1024byte);实际视觉效果可在100HZ下正常显示
 *
 * @details
 *
 */
void disp_test_fram_rate(void)
{
	nrf_gpio_cfg_output(20);

	while(1) {
		nrf_gpio_pin_toggle(20);

		oled_full_fill_test();

		nrf_delay_ms(10);
	}

}


void disp_test_ani(void)
{
	//    display_single_element(0, 0, &baidu_icon,NONE);
	//    display_implement();
	display_init();

	disp_test_fram_rate();
}



/**@brief 将显存的一块区域刷成空白
 *
 * @param [in] pos_x,pos_y 起点坐标(左上角原点)
 *
 *        [in] size_x,size_y 宽和高
 */
void display_blank_icon(int8_t pos_x, int8_t pos_y, uint8_t size_x, uint8_t size_y)
{
	const sBITMAP* code_src;

	if(size_x == 16 && size_y == 9) {

	}

	display_single_element(pos_x, pos_y, code_src, NONE);
}

void display_open(void)
{
	oled_open();
}

void display_close()
{
	oled_close();
}

void display_implement(void)
{
	oled_Update();

	display_open();
}

void display_implement_without_clear(void)
{
	oled_Update_without_clear();

	display_open();
}

APP_TIMER_DEF(m_animation_timer_id);

#define ANIMATION_TEXT_NUM 1

static uint8_t cur_running_text_num = 0;
static uint8_t animation_text_buf[ANIMATION_TEXT_NUM][256];
static animation_text_block_t animation_text_block_buf[ANIMATION_TEXT_NUM];
static animation_icon_block_t animation_icon_set;
static animation_ctrl_t animation_ctrl_msg;


bool is_on_scrolling(void)
{
	return animation_ctrl_msg.ani_running;
}

static void animation_implement(uint16_t frame_index)
{

	//    NRF_LOG_RAW_INFO("animation_timer_frame_index:%d,total_frame_num:%d\r\n",animation_ctrl_msg.cur_frame_index,animation_ctrl_msg.frame_num);

	int16_t c_pos_x;
	for(uint8_t i = 0; i < cur_running_text_num; i++) {
		c_pos_x = animation_text_block_buf[i].start_pos_x - (frame_index % animation_ctrl_msg.frame_num) * ANIMATION_STEP_PIXELS;

		//        NRF_LOG_RAW_INFO("======frame[%3d]cur_pos_x:%d\r\n",frame_index,c_pos_x);
#ifdef USE_EXTERNAL_FONT
		display_utf8(c_pos_x, animation_text_block_buf[i].start_pos_y, \
		             animation_text_block_buf[i].interval, animation_text_block_buf[i].src_data, animation_text_block_buf[i].data_num);
#else
		display_ascii(c_pos_x, animation_text_block_buf[i].start_pos_y, \
		              animation_text_block_buf[i].src_data, animation_text_block_buf[i].data_num);
#endif
	}

	if(animation_icon_set.display_object != 0) {
		display_icon(animation_icon_set.start_pos_x, animation_icon_set.start_pos_y, animation_icon_set.display_object, animation_icon_set.icon_type, animation_icon_set.icon_index);

		if(animation_icon_set.adjust_rate == 0 || frame_index % animation_icon_set.adjust_rate == 0) {
			animation_icon_set.icon_index++;
			animation_icon_set.icon_index %= animation_icon_set.icon_scope;
		}
	}

	if(animation_ctrl_msg.running_handler != NULL) {
		animation_ctrl_msg.running_handler();
	}

	display_implement();

}

static void animation_display_timer_handler(void* p_context)
{
	uint32_t err_code;

	uint32_t timer_speed = 1000 / animation_ctrl_msg.frame_rate;

	//    NRF_LOG_RAW_INFO(">>>>>>>animation_past_time:%d,animation_duration:%d\r\n",animation_ctrl_msg.cur_frame_index*timer_speed ,animation_ctrl_msg.duration);

	if( animation_ctrl_msg.cur_frame_index * timer_speed >= animation_ctrl_msg.duration ) {
		stop_animation();

		if(animation_ctrl_msg.complete_handler != NULL) {
			animation_ctrl_msg.complete_handler();
		}
	} else {
		animation_implement(animation_ctrl_msg.cur_frame_index);
		animation_ctrl_msg.cur_frame_index++;

		err_code = app_timer_start(m_animation_timer_id, APP_TIMER_TICKS(timer_speed), NULL);
		APP_ERROR_CHECK(err_code);

	}

}

void init_oled_display_timer(void)
{
	uint32_t err_code;

	err_code = app_timer_create(&m_animation_timer_id, APP_TIMER_MODE_SINGLE_SHOT, animation_display_timer_handler);
	APP_ERROR_CHECK(err_code);

}

void display_animation_image(animation_ctrl_t* ctrl)
{
	uint32_t err_code;

	ctrl->duration -= ctrl->duration % (1000 / ctrl->frame_rate); // 确保是完整的动画序列

	animation_ctrl_msg = *ctrl;

	uint16_t speed_ms = (1000 / animation_ctrl_msg.frame_rate);

	animation_implement(animation_ctrl_msg.cur_frame_index);
	animation_ctrl_msg.cur_frame_index++;

	err_code = app_timer_start(m_animation_timer_id, APP_TIMER_TICKS(speed_ms), NULL);
	APP_ERROR_CHECK(err_code);
}

void reset_animation_text(void)
{
	cur_running_text_num = 0;
}

/**@brief 设置文本动画
 *
 * @details
 *
 */
void set_animation_text(animation_text_block_t* text)
{
	if(text->data_num == 0) {
		// INVALID PARAM
		return;
	}

	if(cur_running_text_num >= ANIMATION_TEXT_NUM) {
		// NO_MEMORY
		return;
	}

	animation_text_block_buf[cur_running_text_num] = *text;

	animation_text_block_buf[cur_running_text_num].src_data = animation_text_buf[cur_running_text_num];

	memcpy(animation_text_block_buf[cur_running_text_num].src_data, text->src_data, text->data_num);

	cur_running_text_num++;
}

/**@brief 设置动画icon
 *
 * @details
 *
 */
void set_animation_icon(animation_icon_block_t* icon_set)
{
	animation_icon_set = *icon_set;
}


/**@brief 停止动画
 *
 * @details
 *
 */
void stop_animation(void)
{
	uint32_t err_code;

	animation_ctrl_msg.ani_running = false;

	cur_running_text_num = 0;

	err_code = app_timer_stop(m_animation_timer_id);
	APP_ERROR_CHECK(err_code);
}

