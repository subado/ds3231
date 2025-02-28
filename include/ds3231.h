#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/i2c_master.h"
#include <stdint.h>
#include <time.h>
#include <stdbool.h>

typedef void *ds3231_handle_t;

#define DS3231_I2C_ADDRESS 0x68 /*!< I2C address */

#define DS3231_BCD2DEC(bcd, dec) (dec = bcd.tens * 10 + bcd.ones)
#define DS3231_DEC2BCD(dec, bcd)     \
	{                            \
		bcd.tens = (x / 10); \
		bcd.ones = (x % 10); \
	}

typedef union {
	uint8_t data; /*!< Native type representation of the data.  */
	struct {
		uint8_t ones : 4; /*!< 1's digit, represented as BCD */
		uint8_t tens : 4; /*!< 10's digit, represented as BCD */
	};
} ds3231_general_t;

typedef union {
	uint8_t data; /*!< Native type representation of the data.  */
	struct {
		uint8_t ones : 4; /*!< 1's digit, represented as BCD */
		uint8_t tens : 1; /*!< 10's digit, represented as BCD */
		bool pm_flag_twenties : 1; /*!< for 12-hour mode AM/PM bit with true being PM
					    * for 20-hour mode 20's digit */
		bool h12_flag : 1; /*!< 12-(true) or 24-hour(false) mode select bit  */
	};
} ds3231_hours_t;

typedef union {
	uint8_t data; /*!< Native type representation of the data.  */
	struct {
		uint8_t ones : 4; /*!< 1's digit, represented as BCD */
		uint8_t tens : 3; /*!< 10's digit, represented as BCD */
		bool century_flag : 1; /*!< Century flag is toggled when
					* the years overflows from 99 to 00 */
	};

} ds3231_month_t;

typedef struct {
	ds3231_general_t seconds; /*!< In range [0, 59] */
	ds3231_general_t minutes; /*!< In range [0, 59] */
	ds3231_hours_t hours; /*!< In range [1, 12] or [0, 23] */
	uint8_t day; /*!< In range [1, 7] */
	ds3231_general_t date; /*!< In range [1, 31] */
	ds3231_month_t month; /*!< In range [1, 12] */
	ds3231_general_t year; /*!< In range [0, 99] */
} ds3231_raw_timeinfo_t;

/**
 * @brief Create and init clock object
 *
 * @param[in]  i2c_bus    I2C bus handle. Obtained from i2c_new_master_bus()
 * @param[in]  dev_addr   I2C device address of clock
 * @param[out] handle_ret Handle to created DS3231 driver object
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_NO_MEM Not enough memory for the driver
 *     - ESP_ERR_NOT_FOUND Clock not found on the I2C bus
 *     - Others Error from underlying I2C driver
 */
esp_err_t ds3231_create(i2c_master_bus_handle_t i2c_bus, const uint8_t dev_addr,
			ds3231_handle_t *handle_ret);

/**
 * @brief Delete and release a clock object
 *
 * @param clock object handle of ds3231
 */
void ds3231_delete(ds3231_handle_t clock);

/**
 * @brief Read raw time info
 *
 * @param clock object handle of ds3231
 * @param raw time info
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t ds3231_get_raw_timeinfo(ds3231_handle_t clock,
				  ds3231_raw_timeinfo_t *raw_timeinfo);

/**
 * @brief Read time info
 *
 * @param clock object handle of ds3231
 * @param time info
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t ds3231_get_timeinfo(ds3231_handle_t clock, struct tm *timeinfo);
