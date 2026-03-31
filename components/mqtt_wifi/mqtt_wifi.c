#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <mqtt_client.h>

#include "mqtt_wifi.h"
#include "system_manage.h"
#include "setup_wifi.h"
#include "esp_wifi.h"
#include "gpio_config.h"

// define global variables
static const char *TAG = "MQTT_WIFI";
static char mqtt_client_id[64] = "";
static char *mqtt_address = "mqtts://b89cc7c8dfe0427c9dbbe952fc9c426a.s1.eu.hivemq.cloud:8883";
static char *mqtt_username = "Haidanghaui";
static char *mqtt_password = "Haidang2004";
static char json[1024] = "";
static char buffer[1024] = "";
static char cmd[256];
static bool first_pub_version = false;

static int count = 0;
RTC_DATA_ATTR static esp_mqtt_client_handle_t client;
// define extern variables
bool mqtt_connected = false;

static esp_err_t mqtt_subscribe(esp_mqtt_client_handle_t client_id, const char *topic, int qos)
{
    int msg_id = esp_mqtt_client_subscribe_single(client_id, topic, qos);
    ESP_LOGI(TAG, "Subscribed to topic %s with message ID: %d", topic, msg_id);
    if (msg_id == -1)
        return ESP_FAIL;
    else
        return ESP_OK;
}
static esp_err_t mqtt_publish(esp_mqtt_client_handle_t client_id, const char *topic, const char *data, int len, int qos, int retain)
{
    int msg_id = esp_mqtt_client_publish(client_id, topic, data, len, qos, retain);
    ESP_LOGI(TAG, "Published message with ID: %d", msg_id);
    if (msg_id == -1)
        return ESP_OK;
    else
        return ESP_FAIL;
}
void mqtt_publish_data(char *data, char *topic)
{
    // snprintf(mqtt_client_id, sizeof(mqtt_client_id), "%s_%s",DEVICE_NAME,device_name);
    if (client == NULL && client)
    {
        ESP_LOGE(TAG, "MQTT client is not initialized");
        return;
    }
    else
        ESP_LOGI(TAG, "MQTT publish");

    //  printf("client: %p\r\n",(char*)client);
    esp_mqtt_client_publish(client, topic, data, strlen(data), 1, 0);
    // ESP_ERROR_CHECK(mqtt_publish(client,topic,data,strlen(data),1,0));
}
static void check_wifi()
{
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);

    if (mode == WIFI_MODE_AP)
    {
        ESP_LOGI("WIFI", "Đang ở chế độ Access Point");
        wifi_state = 2;
    }
    else if (mode == WIFI_MODE_STA)
    {
        ESP_LOGI("WIFI", "Đang ở chế độ Station");
        reopen_network();
    }
    else if (mode == WIFI_MODE_APSTA)
    {
        ESP_LOGI("WIFI", "Đang ở chế độ AP + STA ");
        wifi_state = 2;
    }
    else
    {
        ESP_LOGI("WIFI", "Wi-Fi đang tắt hoặc không xác định");
    }
}
void mqtt_resubscribe()
{
    if (client == NULL && client)
    {
        ESP_LOGE(TAG, "MQTT client is not initialized");
        return;
    }
    else
        ESP_LOGI(TAG, "MQTT resubscribe");

    mqtt_subscribe(client, SUB_TOPIC, 1);
}
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    // // client = event->client;
    //  printf("client: %p\r\n",( char * )client);
    // your_context_t *context = handler_args;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        // Subscribe to a topic upon successful connection
        exit_accesspoint();

        // snprintf(cmd, sizeof(cmd), "%s/Test", device_name);
        mqtt_subscribe(client, SUB_TOPIC, 1);
        wifi_state = 1;
        mqtt_connected = true;
        s_connected = true;
        act_handle = false;
        // scanned = false;
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        count++;
        mqtt_connected = false;
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // if (first_pub_version == false)
        // {
        //     mqtt_publish_version(HW_VERSION, FW_VERSION, 0);
        //     first_pub_version = true;
        // }
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        memcpy(buffer, event->data, event->data_len);
        buffer[event->data_len] = '\0';
        
   //     convert_to_json_update(buffer);
        // parse_json(buffer);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        check_wifi();
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}
void mqtt_init(char *mqtt_address, char *client_id, char *username, char *password)
{
    ESP_LOGI(TAG, "MQTT configuration function called");
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = mqtt_address,
        .credentials.client_id = client_id,
        .credentials.username = username,
        .credentials.authentication.password = password,
        .network.reconnect_timeout_ms = 5000,
        .session.keepalive = 120,
        .broker.verification.certificate = cert_pem,
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void mqtt_start()
{
    snprintf(mqtt_client_id, sizeof(mqtt_client_id), "SmartHome_Danghaui");
    mqtt_init(mqtt_address, mqtt_client_id, mqtt_username, mqtt_password);
}

void publish_humidity_temperature(unsigned int cmd,unsigned int humidity, float temperature)
{
    snprintf(json, sizeof(json),"{\n"
                                "  \"cmd\": %d,\n"
                                "  \"data\": {\n"
                                "    \"temp\": %.1f,\n"
                                "    \"humi\": %d\n"
                                "  }\n"
                                "}", cmd, temperature, humidity);
    mqtt_publish_data(json, PUB_TOPIC);
}