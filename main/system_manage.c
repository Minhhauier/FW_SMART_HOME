#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "system_manage.h"
#include "setup_wifi.h"
#include "gpio_config.h"
#include "mqtt_wifi.h"      

void system_manage_task(void *arg)
{
    setup_wifi_init();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    while (got_ip==false)
    {
        if(act_handle==false && mqtt_connected==false){
            if(!got_ip){
                printf("==Try connect saved wifi\r\n");
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                // if(got_ip)  break;
                scan_wifi_to_connect();
            }
            //try_connect_saved();
            //count=0;
        }
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI("SYSTEM", "Connected to WiFi, starting MQTT...");
    mqtt_start();
    while (1)
    {
        if(act_handle==false && mqtt_connected==false){
            //try_connect_saved();
            if(!got_ip)
            {
                printf("==Try connect saved wifi\r\n");
                scan_wifi_to_connect();
            }
            //count=0;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}