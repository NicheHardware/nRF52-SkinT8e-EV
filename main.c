
#include <stdint.h>
#include <string.h>

#include "platform.h"
#include "uevent.h"

#include "timedate.h"

#include "led_drv.h"
#include "peripheral_drv.h"

#include "app_timer.h"

const uint8_t led_blink[2] = {1, 1};
void log_on_uevt_handler(uevt_t* evt)
{
	static uint32_t sec = 0;
	switch(evt->evt_id) {
		case UEVT_RTC_1HZ:
			// LOG_RAW("tick - %d\n", sec);
			sec += 1;
			break;
		case UEVT_RTC_8HZ:

			break;
	}
}

// 145 = 4.2v, 460 = 3.7v
static int16_t bat_adc[8];
static uint8_t bat_adc_p = 0;
static bool bat_adc_start = false;
void bat_adc_routine(void)
{
	uint16_t bat_mean;
	uint16_t bat_value;		// 0-105
	if(!bat_adc_start) {
		bat_adc_start = true;
		for (int i = 0; i < 8; ++i) {
			bat_adc[bat_adc_p] = adc_get(2);
			bat_adc_p = (bat_adc_p + 1) & 0x7;
		}
	} else {
		bat_adc[bat_adc_p] = adc_get(2);
		bat_adc_p = (bat_adc_p + 1) & 0x7;
	}
	bat_mean = 0;
	for (int i = 0; i < 8; ++i) {
		bat_mean += bat_adc[i];
	}
	bat_mean /= 8;
	if(bat_mean > 500 || (bat_value == 0 && bat_mean > 480)) {
		bat_value = 0;
	} else if(bat_mean > 460 || (bat_value == 10 && bat_mean > 440)) {
		bat_value = 10;
	} else if(bat_mean > 425 || (bat_value == 20 && bat_mean > 410)) {
		bat_value = 20;
	} else if(bat_mean > 390 || (bat_value == 30 && bat_mean > 375)) {
		bat_value = 30;
	} else if(bat_mean > 360 || (bat_value == 40 && bat_mean > 350)) {
		bat_value = 40;
	} else {
		bat_value = 50;
	}
}

bool bt_connected = false;
uint8_t link_timeout = 0;
void shutdown_now(void)
{
	app_timer_stop_all();
	led_off();
	platform_powerdown(true);
}

static float math_ln(float x)
{
	const float ln10 = 2.302585092994;
	float y, ys;
	float ite = 1;
	float output = 0;
	int8_t k = 0;
	while(x > 1) {
		k += 1;
		x /= 10;
	}
	while(x <= 0.1) {
		k -= 1;
		x *= 10;
	}
	y = (x - 1) / (x + 1);
	ys = y * y;
	for (uint8_t i = 0; i < 13; i++) {
		output += ite / (1 + i * 2);
		ite *= ys;
	}
	output *= 2 * y;
	return output + k * ln10;
}

float calc_temp(float adc)
{
	const uint16_t B = 3928;
	const float t0 = 273.15 + 37;
	const float U0 = 3.3;
	const float Rf = 30000;
	float Ut = (adc * 0.6 / 2048) + 1.46667;
	return B * t0 / (math_ln(Ut / (U0 - Ut)) * t0 + B) - 273.15;
}

char* temp2str(float f)
{
	static char float_str[16];
	uint8_t p = 0;
	uint16_t n;
	f *= 100;
	n = f;

	float_str[p++] = (n / 10000) % 10 + '0';
	float_str[p++] = (n / 1000) % 10 + '0';
	float_str[p++] = (n / 100) % 10 + '0';
	float_str[p++] = '.';
	float_str[p++] = (n / 10) % 10 + '0';
	float_str[p++] = (n / 1) % 10 + '0';
	float_str[p++] = 0;
	return float_str;
}


#define kalman_K (0.08)
void test_handler(uevt_t* evt)
{
	static int16_t x_buffer[8] = {0};
	static int16_t y_buffer[8] = {0};
	static uint8_t buffer_p = 0;
	int16_t x = 0;
	int16_t y = 0;
	switch(evt->evt_id) {
		case UEVT_BTN_DOWN:
		case UEVT_STICK_UP:
		case UEVT_STICK_DOWN:
		case UEVT_STICK_LEFT:
		case UEVT_STICK_RIGHT:
			link_timeout = 0;
			break;
		case UEVT_RTC_1HZ:
			link_timeout += 1;
			if(link_timeout > 60) {
				// shutdown_now();
			}
			led_start(led_blink, 1);
			bat_adc_routine();
			break;
		case UEVT_RTC_8HZ:
			// oled_comp_draw_test();
			break;
		case UEVT_ADC_NEWDATA:
			x_buffer[buffer_p] = ((int16_t*)(evt->content))[0];
			y_buffer[buffer_p] = ((int16_t*)(evt->content))[1];
			buffer_p = (buffer_p + 1) & 0x7;
			static float temp = 0;
			temp = kalman_K * ((int16_t*)(evt->content))[0] + (1 - kalman_K) * temp;
			do {
				static uint8_t tic = 0;
				float tf = calc_temp(temp);
				if(tic++ >= 5) {
					tic = 0;
					LOG_RAW("Temp:%d=[", temp);
					if(temp >= 4090) {
						LOG_RAW("LOW\n");
					} else {
						LOG_RAW("%s\n", temp2str(tf));
					}
				}
			} while(0);
			break;
		case UEVT_BTN_LONG:
			shutdown_now();
			break;
	}
}

APP_TIMER_DEF(ADC_TIMER);
void adc_25hz_handler(void* p_context)
{
	static int16_t p_data[2];
	int16_t voltY = 0;
	int16_t voltX = 0;
	// voltY = adc_get(0);
	voltX = adc_get(0);
	p_data[0] = voltX;
	p_data[1] = voltY;
	uevt_bc(UEVT_ADC_NEWDATA, p_data);
}

void user_event_dispatcher(uevt_t evt)
{
	log_on_uevt_handler(&evt);
	btn_on_uevt_handler(&evt);
	led_on_uevt_handler(&evt);
	test_handler(&evt);
}

void rtc_1hz_handler(void)
{
	uevt_bc_e(UEVT_RTC_1HZ);
}

void rtc_8hz_isr(uint8_t tick)
{
	uevt_bc_e(UEVT_RTC_8HZ);
}

void user_init(void)
{
	app_timer_init();
	app_timer_create(&ADC_TIMER, APP_TIMER_MODE_REPEATED, adc_25hz_handler);
	app_timer_start(ADC_TIMER, APP_TIMER_TICKS(40), NULL);
}

int main(void)
{
	platform_init();
	user_init();

	LOG_RAW("RTT Started.\n");

	for (;;) {
		platform_scheduler();
	}
}


