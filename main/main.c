#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "setup_wifi.h"
#include "gpio_config.h"
#include "system_manage.h"
#include "dht11_sensor.h"

void app_main(void)
{
    xTaskCreate(detect_wifi_task, "detect_wifi_task", 1024 * 4, NULL, 10, NULL);
    xTaskCreate(system_manage_task, "system_manage_task", 1024 * 8, NULL, 10, NULL);
    xTaskCreate(dht11_sensor_task, "dht11_sensor_task", 1024 * 4, NULL, 10, NULL);
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
