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
		"platforms/esp/32/clockless_rmt_esp32.cpp"
		)

idf_component_register(SRCS "${srcs}"
			INCLUDE_DIRS "." "./hal" )

# target_compile_options(${COMPONENT_LIB} PRIVATE "-DESP32") won't work because the esp-idf uses a custom component system
idf_build_set_property(COMPILE_OPTIONS -DESP32 APPEND)
idf_build_set_property(COMPILE_OPTIONS -DFASTLED_NO_PINMAP APPEND)
if (CONFIG_FASTLED_ESP32_I2S)
	idf_build_set_property(COMPILE_OPTIONS "-DFASTLED_ESP32_I2S" APPEND)
endif()
