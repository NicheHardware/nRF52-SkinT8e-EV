
#ifndef _OLED_DISPLAY_DRIVER_H
#define _OLED_DISPLAY_DRIVER_H

#include "bitmap.h"
#include "oled_vram.h"
//#include "font_extract_driver.h"
#include "hal_oled_ssd1306_128x64.h"

#define POS_H_MAX	127
#define POS_L_MAX	63

#define OLED_DRAW_WIDTH	    128
#define OLED_DRAW_HEIGHT	64

//#define USE_EXTERNAL_FONT

#define MAX_UNICODE_BUF_LEN	80

typedef enum {
	ICON_TYPE_STEP_RUN = 0,
	ICON_TYPE_STEP_WALK,
	ICON_TYPE_STEP_RUN_FAST,
	ICON_TYPE_HEART,
	ICON_TYPE_CAMERA,

	ICON_TYPE_BT_ADV,
	ICON_TYPE_BT_CONN,
	ICON_TYPE_BT_DIS_CONN,
	ICON_TYPE_MSG,
	ICON_TYPE_CALL,
	ICON_TYPE_ALARM,
	ICON_TYPE_LOW_POWER_40,
	ICON_TYPE_LOW_POWER_20,
	ICON_TYPE_LACK_POWER,
	ICON_TYPE_CHARGING,

	ICON_TYPE_BT_STATE_CONN,
	ICON_TYPE_BT_STATE_DIS_CONN,
	ICON_TYPE_HEART_UNIT,
	ICON_TYPE_WEEK_NO,
	ICON_TYPE_WEEK_YES,
	ICON_TYPE_BLANK,
	ICON_TYPE_OTA,
	ICON_TYPE_MANUFACTURE,

	ICON_TYPE_HEART_RUNNING_ANI,
	ICON_TYPE_HEART_DETECTING_ANI,

} icon_type_t;

typedef enum {
	DIGIT_SIZE_10X23_1,
	DIGIT_SIZE_11X23_2,
	DIGIT_SIZE_10X23_3,
	DIGIT_SIZE_8X19_1,
	DIGIT_SIZE_9X19_2,
	DIGIT_SIZE_8X19_3,
	DIGIT_SIZE_4X9,

	DIGIT_SIZE_BAR_18X19,
	DIGIT_SIZE_COLON_2X23_1,
	DIGIT_SIZE_COLON_1X23_2,
	DIGIT_SIZE_COLON_1X23_3,

} digit_size_type_t;

void display_init(void);

void display_single_element(int16_t pos_h, int16_t pos_l, const sBITMAP* src, rotate_type_t type);

void display_digit_value(int16_t start_pos_h, int16_t start_pos_l, int16_t end_pos_h, uint8_t interval, digit_size_type_t digit_type, uint32_t in_value);

void display_single_digit(int16_t pos_h, int16_t pos_l, digit_size_type_t type, uint8_t c_value);
#ifdef USE_EXTERNAL_FONT
	void display_flash_font_character(int16_t pos_h, int16_t pos_l, flash_font_type_t flash_font_type, uint32_t UNICODE);

	void convert_ascii_to_unicode(uint32_t* unicode_buf, uint8_t* ascii_buf, uint16_t len);

	void display_flash_font_string(int16_t start_pos_h, int16_t start_pos_l, uint8_t interval, flash_font_type_t flash_font_type, uint32_t* UNICODE_BUF, uint8_t len);

	void convert_utf8_to_unicode(uint32_t* unicode_buf, uint16_t* unicode_bufLen, uint8_t* utf8_buf, uint16_t utf8_bufLen);

	flash_font_type_t recognize_language(uint32_t unicode);

	void display_utf8(int16_t start_pos_h, int16_t start_pos_l, uint8_t interval, uint8_t* utf8_buf, uint16_t utf8_bufLen);
	uint16_t compute_utf8_pixels_width(uint8_t* utf8_buf, uint16_t utf8_bufLen, uint8_t interval);
#endif//USE_EXTERNAL_FONT

void display_icon(int8_t start_x, int8_t start_y, uint8_t display_object, icon_type_t icon_type, uint8_t icon_index);
void display_ascii(int16_t pos_h, int16_t pos_l, uint8_t* disp_data_src, uint8_t len);
void display_open(void);
void display_close(void);
void display_implement_without_clear(void);
void display_implement(void);

void display_ascii_big(int16_t pos_h, int16_t pos_v, uint8_t* disp_data_src, uint8_t len);

#define ANIMATION_STEP_PIXELS   4

typedef void(*ani_complete_evt_handler_t)(void);
typedef void(*ani_running_evt_handler_t)(void);

typedef struct {
	// 起始位置
	int16_t start_pos_x;
	int16_t start_pos_y;
	// 数据源
	uint8_t* src_data;
	// 数据的数量
	uint16_t data_num;
	// 间隔
	uint8_t interval;
} animation_text_block_t;

typedef struct {
	// 起始位置
	int16_t start_pos_x;
	int16_t start_pos_y;
	// icon 参数
	uint8_t display_object;
	icon_type_t icon_type;
	uint8_t icon_scope;
	uint8_t icon_index;
	uint8_t adjust_rate;
} animation_icon_block_t;


/* 动画控制信息 */
typedef struct {
	// 帧率：每秒的帧数
	uint16_t frame_rate;
	// 帧数
	uint16_t frame_num;
	// 当前帧编号
	uint16_t cur_frame_index;
	// 播放时长
	uint16_t duration;
	// 动画执行中
	bool ani_running;
	// 动画完成时的handler
	ani_complete_evt_handler_t complete_handler;
	ani_running_evt_handler_t running_handler;

} animation_ctrl_t;


void init_oled_display_timer(void);
void stop_animation(void);
void set_animation_icon(animation_icon_block_t* icon_set);
void set_animation_text(animation_text_block_t* text);
void display_animation_image(animation_ctrl_t* scroll_msg);
void reset_animation_text(void);


#endif//_OLED_DISPLAY_DRIVER_H
