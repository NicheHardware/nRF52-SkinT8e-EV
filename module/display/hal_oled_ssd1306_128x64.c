
#include "nrf_log.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_util_platform.h"
#include "hal_oled_ssd1306_128x64.h"
#include "platform.h"

/*
    12864--0.96英寸的OLED屏幕，以SSD1306为驱动IC

    在SSD1306中的映射地址是COLUMN0--COLUMN127 ; PAGE0--PAGE7

*/

/****************************************
  (0,0)
 	   ----------------------
       ----------------------
       ----------------------
       ----------------------
       ----------------------
       ----------------------
       ----------------------
       ----------------------
                  (127,63);

********************************************/
#define   OLED_START_COLUMN       0  //起始列号
#define   OLED_END_COLUMN           95

//  SSD1306的寄存器地址
#define OLED_CMD_REG                    0x00
#define OLED_DATA_REG                  0x40


void oled_spi_write_cmd(uint8_t* cmd_buf, uint8_t len)
{
	nrf_gpio_pin_clear(OLED_DC_PIN);

	spi_config_oled();

	spi0_trans_advance(cmd_buf, len, NULL, 0);
}

void oled_spi_write_data(uint8_t* data_buf, uint8_t len)
{
	nrf_gpio_pin_set(OLED_DC_PIN);

	spi_config_oled();

	spi0_trans_advance(data_buf, len, NULL, 0);
}



/*
	设置亮度
*/
void set_light_level(uint8_t level)
{
	uint8_t  SendBuf[10];

	SendBuf[0] = 0x81;
	SendBuf[1] = level;
	oled_spi_write_cmd(SendBuf, 2);

}


void  set_page_num(uint8_t Num)
{
	uint8_t  SendBuf[10];

	SendBuf[0] = 0xB0 | (Num & 0x07);
	oled_spi_write_cmd(SendBuf, 1);

}

void  set_column_num(uint8_t Num)
{
	uint8_t  SendBuf[10];

	SendBuf[0] = 0x00 | (Num & 0x0f); /*set lower column address*/
	SendBuf[1] = 0x10 | ((Num >> 4) & 0x0F); /*set higher column address*/
	oled_spi_write_cmd(SendBuf, 2);
}

void _oled_set_cursor(unsigned char x, unsigned char y)
{
	set_column_num(x + OLED_START_COLUMN);
	set_page_num(y);
}

void _oled_datas(unsigned char* data, unsigned char length)
{
	oled_spi_write_data(data, length);
}

// TEST
#if 1
void ssd1306_cmd(uint8_t cmd)
{
	uint8_t cmd_buf[1] = {0};
	cmd_buf[0] = cmd;

	oled_spi_write_cmd(cmd_buf, 1);
}

static void init_12864(void)
{
	ssd1306_cmd(0xae);//--turn off oled panel
	ssd1306_cmd(0x00);//---set low column address
	ssd1306_cmd(0x10);//---set high column address
	ssd1306_cmd(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	ssd1306_cmd(0x81);//--set contrast control register
	ssd1306_cmd(0xcf); // Set SEG Output Current Brightness

	//	ssd1306_cmd(0xa1);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	//	ssd1306_cmd(0xc8);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常

	ssd1306_cmd(0xa0);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	ssd1306_cmd(0xc0);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常

	ssd1306_cmd(0xa6);//--set normal display
	ssd1306_cmd(0xa8);//--set multiplex ratio(1 to 64)
	ssd1306_cmd(0x3f);//--1/64 duty

	ssd1306_cmd(0xd3);//-set display offset    Shift Mapping RAM Counter (0x00~0x3F)
	ssd1306_cmd(0x00);//-not offset

	ssd1306_cmd(0xd5);//--set display clock divide ratio/oscillator frequency
	ssd1306_cmd(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
	ssd1306_cmd(0xd9);//--set pre-charge period

	ssd1306_cmd(0xf1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	//ssd1306_cmd(0x22);
	ssd1306_cmd(0xda);//--set com pins hardware configuration
	ssd1306_cmd(0x12);
	ssd1306_cmd(0xdb);//--set vcomh

	ssd1306_cmd(0x40);//Set VCOM Deselect Level

	ssd1306_cmd(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
	ssd1306_cmd(0x02);//
	ssd1306_cmd(0x8d);//--set Charge Pump enable/disable
	ssd1306_cmd(0x14);//--set(0x10) disable
	ssd1306_cmd(0xa4);// Disable Entire Display On (0xa4/0xa5)
	ssd1306_cmd(0xa6);// Disable Inverse Display On (0xa6/a7)

	oled_fill_value(0);

	//    ssd1306_cmd(0xaf);//--turn on oled panel
}
#else
static void init_6432(void)
{
	//
	uint8_t cmd[10];

	// set display off
	cmd[0] = 0xAE;
	oled_spi_write_cmd(cmd, 1);

	// set display clock divide ratio/ oscillator frequency
	cmd[0] = 0xd5;
	cmd[1] = 0x80;
	oled_spi_write_cmd(cmd, 2);

	// set multiplex ratio
	cmd[0] = 0xA8;
	cmd[1] = 0x1F;
	oled_spi_write_cmd(cmd, 2);

	// set display offset
	cmd[0] = 0xd3;
	cmd[1] = 0x00;
	oled_spi_write_cmd(cmd, 2);

	// set start line
	cmd[0] = 0x40;
	oled_spi_write_cmd(cmd, 1);

	// set charge pump
	cmd[0] = 0x8d;
	cmd[1] = 0x14;
	oled_spi_write_cmd(cmd, 2);

	/* 0xA0 0xC0排线左上角是为起始位，0xA1,0xC8则排线左上角是终止位  */

	// set segment re-map--------左右镜像
	cmd[0] = 0xA1;
	oled_spi_write_cmd(cmd, 1);

	// set COM output scan direction----上下镜像
	cmd[0] = 0xC8;
	oled_spi_write_cmd(cmd, 1);

	// set enconding mode---0XA6设置0为空白，1点亮
	cmd[0] = 0xA6;
	oled_spi_write_cmd(cmd, 1);

	// set COM pins hardware config---屏幕内部GRAM扫描方式，0x12
	cmd[0] = 0xDA;
	cmd[1] = 0x12;
	oled_spi_write_cmd(cmd, 2);

	//set contrast control
	cmd[0] = 0x81;
	cmd[1] = 0xB0;
	oled_spi_write_cmd(cmd, 2);

	// set pre-charge period
	cmd[0] = 0xD9;
	cmd[1] = 0xF1;
	oled_spi_write_cmd(cmd, 2);

	// set VCOMH deselect level
	cmd[0] = 0xDB;
	cmd[1] = 0x40;
	oled_spi_write_cmd(cmd, 2);

	// enable use RAM to display oled
	cmd[0] = 0xA4;
	oled_spi_write_cmd(cmd, 1);

	// 刷屏
	oled_fill_value(0x0);

	oled_close();

}
#endif



void oled_init( void )
{
	// 初始化控制引脚
	//	nrf_gpio_cfg_output( OLED_PWR_PIN );
	nrf_gpio_cfg_output( OLED_RES_PIN );
	nrf_gpio_cfg_output( OLED_DC_PIN );

	// reset
	nrf_gpio_pin_clear( OLED_RES_PIN );
	//wait for power stable
	nrf_delay_ms( 100 );

	// set reset_pin as normal
	nrf_gpio_pin_set( OLED_RES_PIN );
	nrf_delay_ms(1);

	init_12864();
}

void oled_close(void)
{
	uint8_t  SendBuf[10];

	SendBuf[0] = 0xae;
	oled_spi_write_cmd(SendBuf, 1);

	//	nrf_gpio_pin_clear(OLED_PWR_PIN);

	spi_unconfig();
}

void oled_open(void)
{
	uint8_t  SendBuf[10];

	//	nrf_gpio_pin_set(OLED_PWR_PIN);

	SendBuf[0] = 0xaf;
	oled_spi_write_cmd(SendBuf, 1);

	spi_unconfig();
}

/*
	刷屏
*/
void oled_fill_value(uint8_t value)
{
	uint8_t data[128] = {0};

	// get data
	for(uint8_t i = 0; i < 128; i++) {
		data[i] = value;
	}

	for(uint8_t j = 0; j < 8; j++) {
		set_page_num( j );
		set_column_num( 0 );

		oled_spi_write_data(data, 128);
	}
	oled_open();

}


void oled_full_fill_test(void)
{
	uint8_t data[128] = {0};
	uint8_t emt[128] = {0};
	uint8_t full[128] = {0};
	static uint8_t t_index = 0;
	uint8_t value = 0xFF;

	// get data
	for(uint8_t i = 0; i < 128; i++) {
		data[i] = value << (t_index % 8);
		emt[i] = 0xFF;
		full[i] = 0x0;
	}

	for(uint8_t j = 0; j < 8; j++) {
		set_page_num( j );
		set_column_num( 0 );

		if(j == (t_index / 8) ) {
			oled_spi_write_data(data, 128);
		} else if(j > (t_index / 8) ) {
			oled_spi_write_data(emt, 128);
		} else {
			oled_spi_write_data(full, 128);
		}

		oled_open();

	}
	t_index++;

	if(t_index == 1) {
		nrf_delay_ms(500);
	}

	t_index %= 64;

}

// display font form extern flash
#if 0
#include "font_extract_driver.h"

const uint8_t F_NUM_8X16[] = {
	0x00, 0xE0, 0x10, 0x08, 0x08, 0x10, 0xE0, 0x00,
	0x00, 0x0F, 0x10, 0x20, 0x20, 0x10, 0x0F, 0x00, /*"0",0*/
	0x00, 0x10, 0x10, 0xF8, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x00, 0x00, /*"1",1*/
	0x00, 0x70, 0x08, 0x08, 0x08, 0x88, 0x70, 0x00,
	0x00, 0x30, 0x28, 0x24, 0x22, 0x21, 0x30, 0x00, /*"2",2*/
	0x00, 0x30, 0x08, 0x88, 0x88, 0x48, 0x30, 0x00,
	0x00, 0x18, 0x20, 0x20, 0x20, 0x11, 0x0E, 0x00, /*"3",3*/
	0x00, 0x00, 0xC0, 0x20, 0x10, 0xF8, 0x00, 0x00,
	0x00, 0x07, 0x04, 0x24, 0x24, 0x3F, 0x24, 0x00, /*"4",4*/
	0x00, 0xF8, 0x08, 0x88, 0x88, 0x08, 0x08, 0x00,
	0x00, 0x19, 0x21, 0x20, 0x20, 0x11, 0x0E, 0x00, /*"5",5*/
	0x00, 0xE0, 0x10, 0x88, 0x88, 0x18, 0x00, 0x00,
	0x00, 0x0F, 0x11, 0x20, 0x20, 0x11, 0x0E, 0x00, /*"6",6*/
	0x00, 0x38, 0x08, 0x08, 0xC8, 0x38, 0x08, 0x00,
	0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, /*"7",7*/
	0x00, 0x70, 0x88, 0x08, 0x08, 0x88, 0x70, 0x00,
	0x00, 0x1C, 0x22, 0x21, 0x21, 0x22, 0x1C, 0x00, /*"8",8*/
	0x00, 0xE0, 0x10, 0x08, 0x08, 0x10, 0xE0, 0x00,
	0x00, 0x00, 0x31, 0x22, 0x22, 0x11, 0x0F, 0x00, /*"9",9*/
	0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, /*":",10*/

};
uint8_t data_font[32] = {0};

void test_read_flash_font(void)
{
	// read flahs font

	// 读取出8X16的'A'字符应该是{0x00 0x80 0x70 0x08 0x70 0x80 0x00 0x00 0x3C 0x03 0x02 0x02 0x02 0x03 0x3C 0x00 }
	get_ascii_font_from_flash(data_font, FLASH_FONT_ASCII_8X16, 'A');

	uint8_t posx = 0;
	uint8_t posy = 0;
	uint8_t ch = 0;
	uint8_t* data_source;
	uint8_t* write_buf;

	data_source = (uint8_t*)data_font;

	uint8_t draw_buf[8];

	// 高度是16，分开两行写入
	for(uint8_t i = 0; i < 2; i++) {
		_oled_set_cursor(posx, posy + i);
		write_buf = data_source + (ch * 16 + 8 * i);

		// 传到SPI驱动的指针只能是指向RAM区域，如果是const修饰的指针，则在底层会报错
		// 添加一个RAM缓冲
		memcpy(draw_buf, (uint8_t*)write_buf, 8);

		_oled_datas((uint8_t*)draw_buf, 8); // 宽度是8个像素d

	}

	oled_open();


}

void oled_draw_num(uint8_t posx, uint8_t posy, uint8_t ch)
{
	uint8_t* data_source;
	uint8_t* write_buf;

	if(ch > 10) {
		return;
	}

	data_source = (uint8_t*)F_NUM_8X16;

	uint8_t draw_buf[8];

	// 高度是16，分开两行写入
	for(uint8_t i = 0; i < 2; i++) {
		_oled_set_cursor(posx, posy + i);
		write_buf = data_source + (ch * 16 + 8 * i);

		// 传到SPI驱动的指针只能是指向RAM区域，如果是const修饰的指针，则在底层会报错
		// 添加一个RAM缓冲
		memcpy(draw_buf, (uint8_t*)write_buf, 8);

		_oled_datas((uint8_t*)draw_buf, 8); // 宽度是8个像素d

	}
}

#endif
