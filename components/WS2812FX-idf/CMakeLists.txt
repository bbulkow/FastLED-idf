cmake_minimum_required(VERSION 3.5)

set(
    srcs
    "FX.cpp"
    "FX_fcn.cpp"
)

idf_component_register(SRCS "${srcs}"
            INCLUDE_DIRS "."
            REQUIRES FastLED-idf
)