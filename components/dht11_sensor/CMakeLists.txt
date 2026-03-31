idf_component_register(SRCS "dht11_sensor.c"
                    INCLUDE_DIRS include
                    REQUIRES 
                        setup_wifi
                        mqtt_wifi
                        esp_driver_gpio)