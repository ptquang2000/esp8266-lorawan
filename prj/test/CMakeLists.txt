# This is the project CMakeLists.txt file for the test subproject
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "../components")

# For testing lorawan component
# test lorawan ESP_PORT

# For testing sx1278 component
# test sx1278 ESP_PORT

# For testing gateway component
# test gateway ESP_PORT -D WIFI_SSID="WIFI ID" -D WIFI_PASSWORD="WIFI PASSWORD" -D BROKER_URL="MQTT URL"

# For testing end_device component
# test end_device ESP_PORT 

# set(TEST_COMPONENTS "$CACHE{TEST_NAME}" CACHE STRING "List of components to test")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
link_libraries("-u _printf_float -u _scanf_float")
add_compile_definitions(-DMBEDTLS_CMAC_C)
project(lorawan_test)
