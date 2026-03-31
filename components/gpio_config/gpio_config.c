#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <esp_wifi.h>

#include "gpio_config.h"
#include "setup_wifi.h"

static esp_timer_handle_t stop_timer = NULL;

void stop_action_timer_callback(void *arg)
{
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_LOGI("WIFI", "AP interface disabled - Device now in pure Station mode");
    wifi_state=1;
}

void start_stop_timer(void)
{
    if(stop_timer!=NULL){
        esp_timer_stop(stop_timer);
        esp_timer_start_once(stop_timer, 180000000ULL);
        stop_timer = NULL;
    }
    const esp_timer_create_args_t stop_timer_args = {
        .callback = &stop_action_timer_callback,
        .name = "stop_action_timer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&stop_timer_args, &stop_timer));

    // 3 phút = 180 giây = 180 * 1,000,000 micro giây
    ESP_ERROR_CHECK(esp_timer_start_once(stop_timer, 180000000ULL));
    ESP_LOGI("TIMER", "Stop timer started (3 minutes)");
}
void stop_my_timer(void)
{
    if (stop_timer != NULL) {
        esp_timer_stop(stop_timer);
        esp_timer_delete(stop_timer);
        stop_timer = NULL;
        ESP_LOGI("TIMER", "Stop timer manually stopped");
    }
}
void config_gpio_wifi_menu_config(void){
    gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << GPIO_WIFI_CONFIG),      // Select GPIO 
    .mode = GPIO_MODE_INPUT,            // Set as input
    .pull_up_en = GPIO_PULLUP_ENABLE,  // Disable pull-up
    .pull_down_en = GPIO_PULLDOWN_DISABLE,  // Disable pull-down
    .intr_type = GPIO_INTR_DISABLE             
};
gpio_config(&io_conf);

// gpio_install_isr_service(0);
}
void detect_wifi_task(){
    bool config_mode = false;
    config_gpio_wifi_menu_config();
    while (1)
    {
        if(gpio_get_level(GPIO_WIFI_CONFIG)==0)
        {
            while (gpio_get_level(GPIO_WIFI_CONFIG)==0)
            {
                vTaskDelay(500/portTICK_PERIOD_MS);
            }
            wifi_state=2;
            if(config_mode==false)
            {
            reopen_network();
            start_stop_timer();
            config_mode=true;
            }
            else ESP_LOGI("WIFI","Wifi config mode possible");
        }
        if(wifi_state==0){
            gpio_set_level(LED_DECTEC_WIFI,0);
            config_mode=false;
        }
        else if(wifi_state==1){
            gpio_set_level(LED_DECTEC_WIFI,1);
            config_mode=false;
        }
        else{
            gpio_set_level(LED_DECTEC_WIFI,0);
            vTaskDelay(500/portTICK_PERIOD_MS);
            gpio_set_level(LED_DECTEC_WIFI,1);
        }
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
    

}