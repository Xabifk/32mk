idf_component_register(SRCS "joystick.c"
                    INCLUDE_DIRS "include" "${CMAKE_SOURCE_DIR}/main"
                    REQUIRES "ble" "esp_adc_cal")
