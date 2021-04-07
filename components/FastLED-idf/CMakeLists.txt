if(ESP_PLATFORM)
cmake_minimum_required(VERSION 3.5)

set(srcs
        "FastLED.cpp"
        "bitswap.cpp"
        "colorpalettes.cpp"
        "colorutils.cpp"
        "hsv2rgb.cpp"
        "lib8tion.cpp"
        "noise.cpp"
        "platforms.cpp"
        "power_mgt.cpp"
        "wiring.cpp"
        "hal/esp32-hal-misc.c"
        "hal/esp32-hal-gpio.c"
)

# append this if we're using RMT
if(CONFIG_FASTLED_ESP32_DRIVER_RMT)
list(APPEND srcs "platforms/esp/32/clockless_rmt_esp32.cpp")
endif()

idf_component_register(SRCS "${srcs}"
            INCLUDE_DIRS "." "./hal"
)

# needs to be set to keep FastLED happy
target_compile_options(${COMPONENT_LIB} PUBLIC -DESP32)

else()
    message(FATAL_ERROR "FastLED-idf: ESP_PLATFORM is not defined. Try reinstalling ESP-IDF.")
endif() #ESP_PLATFORM