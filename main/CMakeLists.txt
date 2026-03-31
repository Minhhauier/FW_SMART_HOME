idf_component_register(SRCS "main.c" "system_manage.c"
                    INCLUDE_DIRS "."
                    REQUIRES 
                        setup_wifi
                        gpio_config
                        mqtt_wifi
                        dht11_sensor)
