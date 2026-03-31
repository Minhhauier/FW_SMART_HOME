file(GLOB source "*.c")

idf_component_register(
    SRCS ${source}
    INCLUDE_DIRS include
    REQUIRES
        mqtt
        esp_wifi
        # espressif_mqtt
    PRIV_REQUIRES
        main
        setup_wifi

)