
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

typedef enum {
	TEP_STOP,
	TEP_START,
	TEP_NEWDATA,
	TEP_STATBLE,
	TEP_TIMEOUT,
	TEP_ERROR,
} tep_state_t;

static tep_state_t mTepCtrlStatu;
static uint32_t tempValx100, DispTempVal;

static uint8_t strbuf[120] = {0};

void TepDisplayer(void)
{
	char dspstr[50];
	char _strlen;

	static char waittime = 0;

	switch(mTepCtrlStatu) {
		case TEP_STOP: {
			_strlen = sprintf(dspstr, "%s", "Stopped");
		}
		break;
		case TEP_START: {
			_strlen = sprintf(dspstr, "%s", "Start");
			waittime = 0;
		}
		break;
		case TEP_NEWDATA: {
			_strlen = 0;
			for(uint8_t i = 0; i < waittime; i++) {
				_strlen += sprintf(dspstr + _strlen, "%s", ".");
			}
			waittime++;
			waittime %= 8;

		}
		break;
		case TEP_ERROR: {
			_strlen = sprintf(dspstr, "Fail");
		}
		break;

		case TEP_STATBLE: {
			_strlen = sprintf(dspstr, "%d.%d", DispTempVal / 100, DispTempVal % 100);
		}
		break;
	}

	if(_strlen >= 16) {
		_strlen = 16;
	}

	display_ascii((128 - _strlen * 8) / 2, 24, dspstr, _strlen);

	display_ascii(0, 48, strbuf, strlen(strbuf));

	display_ascii(64, 48, strbuf + 60, strlen(strbuf + 60));

	display_implement();

}

void Hello(void)
{
	uint8_t str[] = "Temperature DK";
	display_ascii(8, 24, str, strlen(str));
	// display_ascii_big(0, 0, str, strlen(str));
	display_implement();
}

void displayStr(char* str, uint8_t offset)
{

	strcpy(strbuf + offset, str);
	// display_ascii_big(0, 0, str, strlen(str));
	// display_implement();
}

typedef enum {
	STATE_LACK_DATA,
	STATE_RISE,
	STATE_FALL,
	STATE_STABLE,
	STATE_MESS,
} tep_val_state_t;

#define ALG_DATA_NUM_TH 45
typedef struct {
	uint16_t timeout;
	tep_val_state_t val_state;
	uint32_t stable_val;
	uint32_t max_val;
	uint32_t min_val;
	uint32_t max_idx;
	uint32_t min_idx;
	uint32_t buf[25 * 2];
	uint16_t num;
} filter_t;


filter_t mtepFilter;

void tepFilter_init(void)
{
	memset(&mtepFilter, 0, sizeof(mtepFilter));
	mtepFilter.timeout = 8 * 20;
}

void tepFilter_handler(uint32_t val)
{

	uint8_t nm;
	nm = sizeof(mtepFilter.buf) / sizeof(mtepFilter.buf[0]);

	// 传入工作区buf
	for(uint8_t i = 0; i < nm - 1; i++) {
		mtepFilter.buf[i] = mtepFilter.buf[i + 1];
	}
	mtepFilter.buf[nm - 1] = val;
	// 记录数量
	if(mtepFilter.num < nm) {
		mtepFilter.num++;
	}

	uint32_t _temp;
	mtepFilter.max_val = 0;
	mtepFilter.min_val = 0xFFFFFFFF;

	mtepFilter.min_idx = nm - 1;
	mtepFilter.max_idx = nm - 1;

	int8_t _idx = nm - 1;

	int16_t td_cnt = 0;
	// 记录最大值、最小值
	for(uint8_t k = 0; k < mtepFilter.num ; k++) {

		_temp = mtepFilter.buf[_idx];

		if(_temp >= mtepFilter.max_val + 20) {

			mtepFilter.max_val = _temp;
			mtepFilter.max_idx = _idx;

		} else if(_temp <= mtepFilter.min_val - 20) {

			mtepFilter.min_val = _temp;
			mtepFilter.min_idx = _idx;
		}

		if(_idx > 0) {

			if(mtepFilter.buf[_idx] >= mtepFilter.buf[_idx - 1] + 5) {
				td_cnt++;
			} else if(mtepFilter.buf[_idx] <= mtepFilter.buf[_idx - 1] - 5) {
				td_cnt--;
			}
			_idx--;
		}

	}

	// 记录状态
	if(mtepFilter.num < ALG_DATA_NUM_TH) {
		mtepFilter.val_state = STATE_LACK_DATA;
	} else if(mtepFilter.num == ALG_DATA_NUM_TH) {
		mtepFilter.val_state = STATE_MESS;
	}
	// 监测曲线
	else {

		uint8_t riseNm;
		uint8_t st_idx, ed_idx;
		st_idx = nm - mtepFilter.num;
		ed_idx = nm;

		char _str[8] = {0};
		sprintf(_str, "%d", td_cnt);

		LOG_RAW("td_cnt:%d\r\n", td_cnt);

		displayStr(_str, 60);

		if(td_cnt > 30 || td_cnt < -30) {
			if(td_cnt > 30) {
				mtepFilter.val_state = STATE_RISE;
				displayStr("UP", 0);
			} else if(td_cnt < -30) {
				mtepFilter.val_state = STATE_FALL;
				displayStr("DOWN", 0);
			}
		} else {

			uint32_t sum = 0, avg = 0;
			for(uint8_t i = st_idx; i < ed_idx; i++) {
				sum += mtepFilter.buf[i];
			}
			avg = sum / mtepFilter.num;

			// 偏差求和
			sum = 0;
			for(uint8_t i = st_idx; i < ed_idx; i++) {
				sum += (mtepFilter.buf[i] - avg) * (mtepFilter.buf[i] - avg);
			}

			// LOG_RAW(">>>>>avg:%d diff:%d\r\n", avg, sum);

			if(mtepFilter.val_state != STATE_STABLE) {
				// if(mtepFilter.max_idx==mtepFilter.min_idx || sum<=80)
				{
					mtepFilter.val_state = STATE_STABLE;
					displayStr("Stable", 0);
					mtepFilter.stable_val = avg;
					// LOG_RAW(">>>>>>Stable\r\n");
				}
			}
		}
	}
}
#include "nrf_gpio.h"

void disp_handler(uevt_t* evt)
{
	static char _mask = 0;

	switch(evt->evt_id) {

		case UEVT_DTIME_UPDATE: {
			TepDisplayer();

			if(mtepFilter.timeout != 0) {
				mtepFilter.timeout --;
			}
		}
		break;

		case UEVT_BTN_RELEASE: {
			if(mTepCtrlStatu == TEP_STOP) {
				mTepCtrlStatu = TEP_START;
				DispTempVal = 0;

				//  清除滤波器
				tepFilter_init();
				_mask = 1;

			} else {
				_mask = 0;
				mTepCtrlStatu = TEP_STOP;
			}
		}
		break;

		case UEVT_ADC_NEWDATA_FL: {
			if(_mask == 0) {
				break;
			}
			mTepCtrlStatu = TEP_NEWDATA;

			tepFilter_handler(tempValx100);

			if(mtepFilter.val_state == STATE_STABLE) {

				DispTempVal = mtepFilter.stable_val;

				mTepCtrlStatu = TEP_STATBLE;
			} else if(mtepFilter.timeout == 0) {

				if(mtepFilter.val_state == STATE_RISE) {
					DispTempVal = mtepFilter.max_val;
				} else if(mtepFilter.val_state == STATE_FALL) {
					DispTempVal = mtepFilter.min_val;
				}

				mTepCtrlStatu = TEP_TIMEOUT;
			}

		}
		break;
	}

}

#define kalman_K (0.03)
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
				tempValx100 = tf * 100;
				uevt_bc_e(UEVT_ADC_NEWDATA_FL);

				// if(tic++ >= 5)
				{
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
APP_TIMER_DEF(DISP_TIMER);

void D_timer_handler(void* p_context)
{
	uevt_bc_e(UEVT_DTIME_UPDATE);
}

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
	disp_handler(&evt);
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

	app_timer_create(&DISP_TIMER, APP_TIMER_MODE_REPEATED, D_timer_handler);
	app_timer_start(DISP_TIMER, APP_TIMER_TICKS(40), NULL);

}

int main(void)
{
	platform_init();
	Hello();

	LOG_RAW("RTT Started.\n");

	for (;;) {
		platform_scheduler();
	}
}


