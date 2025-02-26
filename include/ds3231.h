#pragma once

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "driver/i2c_master.h"
#include <stdbool.h>

typedef void *ds3231_handle_t;

#define DS3231_I2C_ADDRESS 0x68 /*!< I2C address */

typedef struct {
	uint8_t seconds; /*!< In range [0, 59] */
	uint8_t minutes; /*!< In range [0, 59] */
	uint8_t hours; /*!< in range [0, 12] if hours_12h_mode or [0, 23] elsewhere */
	bool hours_12h_mode; /*!< 12-(true) or 24-hour(false) mode select bit  */
	bool hours_pm_flag; /*!< AM/PM bit with true being PM  */
	uint8_t day; /*!< In range [1, 7] */
	uint8_t date; /*!< In range [1, 31] */
	uint8_t month; /*!< In range [1, 12] */
	uint8_t year; /*!< In range [0, 99] */
	bool year_century_flag; /*!< Century bit is toggled when
				 * the years overflows from 99 to 00 */
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
