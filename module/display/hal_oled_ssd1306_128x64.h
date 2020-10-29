#ifndef HAL_OLED_SSD1306_128X64_H
#define HAL_OLED_SSD1306_128X64_H
#include <stdint.h>
#include "stdbool.h"
#include "nrf_gpio.h"

#include "nrf_log.h"

//#define ENABLE_LOG_OLED

#ifdef ENABLE_LOG_OLED
	#define SXP_LOG_OLED(...)                  NRF_LOG_INTERNAL_RAW_INFO( __VA_ARGS__)
#else
	#define SXP_LOG_OLED(...)
#endif

void oled_init(void);

void oled_open(void);

void oled_close(void);

void _oled_set_cursor(unsigned char x, unsigned char y);

void _oled_datas(unsigned char* data, unsigned char length);

void set_light_level(uint8_t level);

void  set_page_num(uint8_t num);

void  set_column_num(uint8_t num);

void oled_fill_value(uint8_t value);


void oled_draw_num(uint8_t posx, uint8_t posy, uint8_t ch);

#endif /* HAL_OLED_SSD1306_128X64_H */
