#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "dht11_sensor.h"
#include "mqtt_wifi.h"

#define DHT11_GPIO GPIO_NUM_4
#define DHT11_TIMEOUT_US 120

static const char *TAG = "DHT11";

static bool wait_level(gpio_num_t pin, int level, uint32_t timeout_us)
{
	int64_t start = esp_timer_get_time();
	while (gpio_get_level(pin) != level)
	{
		if ((esp_timer_get_time() - start) > timeout_us)
		{
			return false;
		}
	}
	return true;
}

bool dht11_read(gpio_num_t pin, uint8_t *temperature, uint8_t *humidity)
{
	uint8_t data[5] = {0};

	gpio_set_direction(pin, GPIO_MODE_OUTPUT);
	gpio_set_level(pin, 0);
	vTaskDelay(pdMS_TO_TICKS(20));
	gpio_set_level(pin, 1);
	esp_rom_delay_us(30);
	gpio_set_direction(pin, GPIO_MODE_INPUT);

	if (!wait_level(pin, 0, DHT11_TIMEOUT_US)) return false;
	if (!wait_level(pin, 1, DHT11_TIMEOUT_US)) return false;
	if (!wait_level(pin, 0, DHT11_TIMEOUT_US)) return false;

	for (int i = 0; i < 40; i++)
	{
		int64_t t0;
		int64_t t1;

		if (!wait_level(pin, 1, DHT11_TIMEOUT_US)) return false;
		t0 = esp_timer_get_time();
		if (!wait_level(pin, 0, DHT11_TIMEOUT_US)) return false;
		t1 = esp_timer_get_time();

		data[i / 8] <<= 1;
		if ((t1 - t0) > 40)
		{
			data[i / 8] |= 1;
		}
	}

	if (((uint8_t)(data[0] + data[1] + data[2] + data[3])) != data[4])
	{
		return false;
	}

	*humidity = data[0];
	*temperature = data[2];
	return true;
}

void dht11_sensor_task(void *pvParameters)
{
	(void)pvParameters;
	uint8_t temp;
	uint8_t hum;
    TickType_t last_publish_time = xTaskGetTickCount();

	while (1)
	{
    
		if (dht11_read(DHT11_GPIO, &temp, &hum))
		{
			ESP_LOGI(TAG, "Nhiet do: %u C, Do am: %u %%", temp, hum);
		}
		else
		{
			ESP_LOGW(TAG, "Doc DHT11 that bai");
		}
        if(mqtt_connected)
        {
            if(xTaskGetTickCount() - last_publish_time >= pdMS_TO_TICKS(30000))
            {
                publish_humidity_temperature(101,hum,temp);
                last_publish_time = xTaskGetTickCount();
            }
        }

		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

