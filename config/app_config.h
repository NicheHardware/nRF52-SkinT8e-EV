#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_


// #define DEBUG

// #include "build.h"
#if is_build_release==1
	#define NRF_LOG_ENABLED 0
#else
	#define NRF_LOG_ENABLED 1
#endif

#define NRF_SDH_BLE_PERIPHERAL_LINK_COUNT 1
#define NRF_SDH_BLE_CENTRAL_LINK_COUNT 0
#define NRF_SDH_BLE_TOTAL_LINK_COUNT 1

#define NRF_BLE_QWR_ENABLED 0

#define NRF_LOG_DEFERRED 1	// log buffer

#define WDT_ENABLED 1

#define APP_TIMER_ENABLED 1

//#define NRF_BLE_GATT_MAX_MTU_SIZE 23

//#define PEER_MANAGER_ENABLED 1

#define PWM_ENABLED 1
#define PWM0_ENABLED 1
#define PWM1_ENABLED 1
#define PWM2_ENABLED 1
#define APP_PWM_ENABLED 1

#define BLE_BAS_ENABLED 1
#define BLE_DIS_ENABLED 1
#define BLE_HIDS_ENABLED 1
#define BLE_ANCS_C_ENABLED 1
#define BLE_DFU_ENABLED 1
#define NRF_DFU_SVCI_ENABLED
#define NRF_DFU_TRANSPORT_BLE 1

#define BLE_DB_DISCOVERY_ENABLED 1
#define NRF_BLE_GQ_ENABLED 1
#define NRF_BLE_GQ_GATTC_WRITE_MAX_DATA_LEN (64)
#define NRF_BLE_GQ_GATTS_HVX_MAX_DATA_LEN (96)
// #define APP_FIFO_ENABLED 1

#define NRFX_RTC_ENABLED 1
#define NRFX_RTC2_ENABLED 1

#define RTC_ENABLED 1
#define RTC2_ENABLED 1

#define APP_TIMER_CONFIG_USE_SCHEDULER 1	// hid工程自带
#define CRC16_ENABLED 1	// hid工程自带
#define FDS_ENABLED 1

// #define RTC_CONFIG_LOG_ENABLED 1

#define SPI_ENABLED 1
#define SPI0_ENABLED 1
#define SPI0_USE_EASY_DMA 0

#define TWI_ENABLED 0
#define TWI1_ENABLED 0
#define TWI1_USE_EASY_DMA 0

#define GPIOTE_ENABLED 1
#define GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS 8
#define APP_GPIOTE_ENABLED 1

#define BUTTON_ENABLED 1

// TODO:
// 移除apply_old_config, 清理sdk_config的旧config, 全部使用新的define
#define SAADC_ENABLED 1
// #define NRFX_SAADC_ENABLED 1

// #define BLE_ANCS_C_ENABLED 1
// #define BLE_DB_DISCOVERY_ENABLED 1
#define FSTORAGE_ENABLED 1

// #define SPIM_NRF52_ANOMALY_109_WORKAROUND_ENABLED 1
// #ifndef CONFIG_NFCT_PINS_AS_GPIOS
#define CONFIG_NFCT_PINS_AS_GPIOS 1
// #endif

#define UART_ENABLED 0
// #define BUTTON_ENABLED 0

#ifndef TIMER_ENABLED
	#define TIMER_ENABLED 1
#endif

#ifndef TIMER0_ENABLED
	#define TIMER0_ENABLED 1
#endif

#ifndef TIMER1_ENABLED
	#define TIMER1_ENABLED 1
#endif

#ifndef TIMER_DEFAULT_CONFIG_FREQUENCY
	#define TIMER_DEFAULT_CONFIG_FREQUENCY 9
#endif

#ifndef TIMER_DEFAULT_CONFIG_MODE
	#define TIMER_DEFAULT_CONFIG_MODE 0
#endif

#ifndef TIMER_DEFAULT_CONFIG_BIT_WIDTH
	#define TIMER_DEFAULT_CONFIG_BIT_WIDTH 3
#endif

#ifndef TIMER_DEFAULT_CONFIG_IRQ_PRIORITY
	#define TIMER_DEFAULT_CONFIG_IRQ_PRIORITY 7
#endif

#define NRF_LOG_BACKEND_UART_ENABLED 0
#define NRF_LOG_BACKEND_RTT_ENABLED 1
#define NRF_LOG_BACKEND_SERIAL_USES_UART 0
#define NRF_LOG_BACKEND_SERIAL_USES_RTT 1

// <e> NRF_BLE_SCAN_ENABLED - nrf_ble_scan - Scanning Module
//==========================================================
#ifndef NRF_BLE_SCAN_ENABLED
	#define NRF_BLE_SCAN_ENABLED 1
#endif
// <o> NRF_BLE_SCAN_BUFFER - Data length for an advertising set.
#ifndef NRF_BLE_SCAN_BUFFER
	#define NRF_BLE_SCAN_BUFFER 31
#endif

// <o> NRF_BLE_SCAN_NAME_MAX_LEN - Maximum size for the name to search in the advertisement report.
#ifndef NRF_BLE_SCAN_NAME_MAX_LEN
	#define NRF_BLE_SCAN_NAME_MAX_LEN 32
#endif

// <o> NRF_BLE_SCAN_SHORT_NAME_MAX_LEN - Maximum size of the short name to search for in the advertisement report.
#ifndef NRF_BLE_SCAN_SHORT_NAME_MAX_LEN
	#define NRF_BLE_SCAN_SHORT_NAME_MAX_LEN 32
#endif

// <o> NRF_BLE_SCAN_SCAN_INTERVAL - Scanning interval. Determines the scan interval in units of 0.625 millisecond.
#ifndef NRF_BLE_SCAN_SCAN_INTERVAL
	#define NRF_BLE_SCAN_SCAN_INTERVAL 160
#endif

// <o> NRF_BLE_SCAN_SCAN_DURATION - Duration of a scanning session in units of 10 ms. Range: 0x0001 - 0xFFFF (10 ms to 10.9225 ms). If set to 0x0000, the scanning continues until it is explicitly disabled.
#ifndef NRF_BLE_SCAN_SCAN_DURATION
	#define NRF_BLE_SCAN_SCAN_DURATION 0
#endif

// <o> NRF_BLE_SCAN_SCAN_WINDOW - Scanning window. Determines the scanning window in units of 0.625 millisecond.
#ifndef NRF_BLE_SCAN_SCAN_WINDOW
	#define NRF_BLE_SCAN_SCAN_WINDOW 80
#endif

// <o> NRF_BLE_SCAN_MIN_CONNECTION_INTERVAL - Determines minimum connection interval in milliseconds.
#ifndef NRF_BLE_SCAN_MIN_CONNECTION_INTERVAL
	#define NRF_BLE_SCAN_MIN_CONNECTION_INTERVAL 7.5
#endif

// <o> NRF_BLE_SCAN_MAX_CONNECTION_INTERVAL - Determines maximum connection interval in milliseconds.
#ifndef NRF_BLE_SCAN_MAX_CONNECTION_INTERVAL
	#define NRF_BLE_SCAN_MAX_CONNECTION_INTERVAL 30
#endif

// <o> NRF_BLE_SCAN_SLAVE_LATENCY - Determines the slave latency in counts of connection events.
#ifndef NRF_BLE_SCAN_SLAVE_LATENCY
	#define NRF_BLE_SCAN_SLAVE_LATENCY 0
#endif

// <o> NRF_BLE_SCAN_SUPERVISION_TIMEOUT - Determines the supervision time-out in units of 10 millisecond.
#ifndef NRF_BLE_SCAN_SUPERVISION_TIMEOUT
	#define NRF_BLE_SCAN_SUPERVISION_TIMEOUT 4000
#endif

// <o> NRF_BLE_SCAN_SCAN_PHY  - PHY to scan on.

// <0=> BLE_GAP_PHY_AUTO
// <1=> BLE_GAP_PHY_1MBPS
// <2=> BLE_GAP_PHY_2MBPS
// <4=> BLE_GAP_PHY_CODED
// <255=> BLE_GAP_PHY_NOT_SET

#ifndef NRF_BLE_SCAN_SCAN_PHY
	#define NRF_BLE_SCAN_SCAN_PHY 1
#endif

// <e> NRF_BLE_SCAN_FILTER_ENABLE - Enabling filters for the Scanning Module.
//==========================================================
#ifndef NRF_BLE_SCAN_FILTER_ENABLE
	#define NRF_BLE_SCAN_FILTER_ENABLE 0
#endif
// <o> NRF_BLE_SCAN_UUID_CNT - Number of filters for UUIDs.
#ifndef NRF_BLE_SCAN_UUID_CNT
	#define NRF_BLE_SCAN_UUID_CNT 0
#endif

// <o> NRF_BLE_SCAN_NAME_CNT - Number of name filters.
#ifndef NRF_BLE_SCAN_NAME_CNT
	#define NRF_BLE_SCAN_NAME_CNT 0
#endif

// <o> NRF_BLE_SCAN_SHORT_NAME_CNT - Number of short name filters.
#ifndef NRF_BLE_SCAN_SHORT_NAME_CNT
	#define NRF_BLE_SCAN_SHORT_NAME_CNT 0
#endif

// <o> NRF_BLE_SCAN_ADDRESS_CNT - Number of address filters.
#ifndef NRF_BLE_SCAN_ADDRESS_CNT
	#define NRF_BLE_SCAN_ADDRESS_CNT 0
#endif

// <o> NRF_BLE_SCAN_APPEARANCE_CNT - Number of appearance filters.
#ifndef NRF_BLE_SCAN_APPEARANCE_CNT
	#define NRF_BLE_SCAN_APPEARANCE_CNT 0
#endif

#endif
