# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)


# run ESP_PORT -D WIFI_SSID="WIFI ID" -D WIFI_PASSWORD="WIFI PASSWORD" -D BROKER_URL="MQTT URL"

set(EXTRA_COMPONENT_DIRS "components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
link_libraries("-u _printf_float -u _scanf_float")
add_compile_definitions(-DMBEDTLS_CMAC_C -DEND_DEVICE)
project(lorawan)
