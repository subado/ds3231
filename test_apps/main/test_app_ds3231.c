#include <stdio.h>
#include "unity.h"
#include "driver/i2c_master.h"
#include "ds3231.h"
#include "esp_system.h"
#include "esp_log.h"
#include "unity.h"
#include "unity_test_runner.h"
#include "unity_test_utils_memory.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Pinout for ESP32-S3
#define I2C_MASTER_SCL_IO 10 /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 9 /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM	  I2C_NUM_0 /*!< I2C port number for master dev */

static const char *TAG = "ds3231 test";
static ds3231_handle_t ds3231 = NULL;
static i2c_master_bus_handle_t i2c_handle = NULL;

static void i2c_bus_init(void)
{
	const i2c_master_bus_config_t bus_config = {
		.i2c_port = I2C_MASTER_NUM,
		.sda_io_num = I2C_MASTER_SDA_IO,
		.scl_io_num = I2C_MASTER_SCL_IO,
		.clk_source = I2C_CLK_SRC_DEFAULT,
	};

	esp_err_t ret = i2c_new_master_bus(&bus_config, &i2c_handle);
	TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, ret, "I2C install returned error");
}

static void i2c_clock_ds3231_init(void)
{
	esp_err_t ret;

	i2c_bus_init();
	ret = ds3231_create(i2c_handle, DS3231_I2C_ADDRESS, &ds3231);
	TEST_ASSERT_EQUAL(ESP_OK, ret);
	TEST_ASSERT_NOT_NULL_MESSAGE(ds3231, "ds3231 create returned NULL");

	TEST_ASSERT_EQUAL(ESP_OK, ret);
}

TEST_CASE("Clock ds3231_get_raw_timeinfo test", "[ds3231]")
{
	esp_err_t ret = ESP_OK;
	ds3231_raw_timeinfo_t raw_timeinfo;

	i2c_clock_ds3231_init();

	ret = ds3231_get_raw_timeinfo(ds3231, &raw_timeinfo);
	TEST_ASSERT_EQUAL(ESP_OK, ret);

	ESP_LOGI(TAG,
		 "%02d-%02d-%02d %02d:%02d:%02d %s, day: %d, century_flag: %d",
		 raw_timeinfo.year, raw_timeinfo.month, raw_timeinfo.date,
		 raw_timeinfo.hours, raw_timeinfo.minutes, raw_timeinfo.seconds,
		 (raw_timeinfo.hours_12h_mode ?
			  (raw_timeinfo.hours_pm_flag ? "PM" : "AM") :
			  ""),
		 raw_timeinfo.day, raw_timeinfo.year_century_flag);

	ds3231_delete(ds3231);
	ret = i2c_del_master_bus(i2c_handle);
	TEST_ASSERT_EQUAL(ESP_OK, ret);
	vTaskDelay(pdMS_TO_TICKS(
		50)); // Give FreeRTOS some time to free its resources
}

#define TEST_MEMORY_LEAK_THRESHOLD (500)

void setUp(void)
{
	unity_utils_set_leak_level(TEST_MEMORY_LEAK_THRESHOLD);
	unity_utils_record_free_mem();
}

void tearDown(void)
{
	unity_utils_evaluate_leaks();
}

void app_main(void)
{
	unity_run_menu();
}
