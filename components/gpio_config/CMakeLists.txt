idf_component_register(SRCS "gpio_config.c"
                    INCLUDE_DIRS include
                    REQUIRES 
                        setup_wifi
                        driver
                        esp_wifi
                        esp_timer)