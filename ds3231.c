#include "esp_err.h"
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "esp_check.h"
#include "ds3231.h"

#define I2C_CLK_SPEED 1000000

/* DS3231 register */
#define DS3231_TIMEINFO_DATA 0x00

/* DS3231 register lenths */
#define DS3231_TIMEINFO_DATA_LEN 7

/* DS3231 register bit masks */
#define DS3231_HOURS_MASK	   0x1f
#define DS3231_HOURS_PM_FLAG_MASK  0x20
#define DS3231_HOURS_12H_MODE_MASK 0x40

#define DS3231_MONTH_MASK   0x1f
#define DS3231_CENTURY_MASK 0x80

#define DS3231_WRITE_BUFF_SIZE (DS3231_TIMEINFO_DATA_LEN + 1)

/*******************************************************************************
* Types definitions
*******************************************************************************/

typedef struct {
	i2c_master_dev_handle_t i2c_handle;
	gpio_num_t int_pin;
} ds3231_dev_t;

/*******************************************************************************
* Function definitions
*******************************************************************************/
static esp_err_t ds3231_write(ds3231_handle_t clock,
			      const uint8_t reg_start_addr,
			      const uint8_t *data_buf, const uint8_t data_len);
static esp_err_t ds3231_read(ds3231_handle_t clock,
			     const uint8_t reg_start_addr, uint8_t *data_buf,
			     const uint8_t data_len);

static uint8_t bcd2dec(uint8_t x);
static uint8_t dec2bcd(uint8_t x);

/* static esp_err_t ds3231_get_raw_value(ds3231_handle_t clock, uint8_t reg, ds3231_raw_value_t *value); */

/*******************************************************************************
* Local variables
*******************************************************************************/
static const char *TAG = "DS3231";

/*******************************************************************************
* Public API functions
*******************************************************************************/

esp_err_t ds3231_create(i2c_master_bus_handle_t i2c_bus, const uint8_t dev_addr,
			ds3231_handle_t *handle_ret)
{
	esp_err_t ret = ESP_OK;

	// Allocate memory and init the driver object
	ds3231_dev_t *clock = (ds3231_dev_t *)calloc(1, sizeof(ds3231_dev_t));
	ESP_RETURN_ON_FALSE(clock != NULL, ESP_ERR_NO_MEM, TAG,
			    "Not enough memory");

	// Add new I2C device
	const i2c_device_config_t i2c_dev_cfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = dev_addr,
		.scl_speed_hz = I2C_CLK_SPEED,
	};
	ESP_GOTO_ON_ERROR(i2c_master_bus_add_device(i2c_bus, &i2c_dev_cfg,
						    &clock->i2c_handle),
			  err, TAG, "Failed to add new I2C device");
	assert(clock->i2c_handle);

	*handle_ret = clock;
	return ret;

err:
	ds3231_delete(clock);
	return ret;
}

void ds3231_delete(ds3231_handle_t clock)
{
	ds3231_dev_t *clck = (ds3231_dev_t *)clock;

	if (clck->i2c_handle) {
		i2c_master_bus_rm_device(clck->i2c_handle);
	}

	free(clck);
}

esp_err_t ds3231_get_raw_timeinfo(ds3231_handle_t clock,
				  ds3231_raw_timeinfo_t *raw_timeinfo)
{
	esp_err_t ret = ESP_FAIL;
	uint8_t data[DS3231_TIMEINFO_DATA_LEN];

	assert(raw_timeinfo != NULL);

	memset(raw_timeinfo, 0, sizeof(data));

	ret = ds3231_read(clock, DS3231_TIMEINFO_DATA, data, sizeof(data));
	if (ret == ESP_OK) {
		uint8_t i = 0;
		raw_timeinfo->seconds = bcd2dec(data[i++]);

		raw_timeinfo->minutes = bcd2dec(data[i++]);

		raw_timeinfo->hours = bcd2dec(data[i] & DS3231_HOURS_MASK);
		raw_timeinfo->hours_12h_mode =
			bcd2dec(data[i] & DS3231_HOURS_12H_MODE_MASK);
		raw_timeinfo->hours_pm_flag =
			bcd2dec(data[i++] & DS3231_HOURS_PM_FLAG_MASK);
		if (!raw_timeinfo->hours_12h_mode &&
		    raw_timeinfo->hours_pm_flag) {
			raw_timeinfo->hours += 20;
		}

		raw_timeinfo->day = bcd2dec(data[i++]);

		raw_timeinfo->date = bcd2dec(data[i++]);

		raw_timeinfo->month = bcd2dec(data[i] & DS3231_MONTH_MASK);
		raw_timeinfo->year_century_flag =
			bcd2dec(data[i++] & DS3231_CENTURY_MASK);

		raw_timeinfo->year = bcd2dec(data[i++]);
	}

	return ret;
}

/*******************************************************************************
* Private functions
*******************************************************************************/

static esp_err_t ds3231_write(ds3231_handle_t clock,
			      const uint8_t reg_start_addr,
			      const uint8_t *data_buf, const uint8_t data_len)
{
	ds3231_dev_t *clck = (ds3231_dev_t *)clock;
	assert(clck);

	assert(data_len < DS3231_WRITE_BUFF_SIZE);
	uint8_t write_buff[DS3231_WRITE_BUFF_SIZE] = { reg_start_addr };
	memcpy(&write_buff[1], data_buf, data_len);
	return i2c_master_transmit(clck->i2c_handle, write_buff, data_len + 1,
				   -1);
}

static esp_err_t ds3231_read(ds3231_handle_t clock,
			     const uint8_t reg_start_addr, uint8_t *data_buf,
			     const uint8_t data_len)
{
	uint8_t reg_buff[] = { reg_start_addr };
	ds3231_dev_t *clck = (ds3231_dev_t *)clock;
	assert(clck);

	/* Write register number and read data */
	return i2c_master_transmit_receive(clck->i2c_handle, reg_buff,
					   sizeof(reg_buff), data_buf, data_len,
					   -1);
}

uint8_t bcd2dec(uint8_t x)
{
	return (x >> 4) * 10 + (x & 0xf);
}

uint8_t dec2bcd(uint8_t x)
{
	return ((x / 10) << 4) | (x % 10);
}
