# This is the project CMakeLists.txt file for the test subproject
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "../components")

# For testing lorawan component
# ./start_tests.sh -p 'ESP PORT' -D TEST_COMPONENTS='lorawan'

# For testing sx1278 component
# ./start_tests.sh -p 'ESP PORT' -D TEST_COMPONENTS='sx1278'

# For testing gateway component
# ./start_tests.sh -p 'ESP PORT' -D TEST_COMPONENTS='gateway' -D WIFI_SSID='WIFI NAME' -D WIFI_PASSWORD='WIFI PASSWORD' -D BROKER_URL='BROKER IP'

# set(TEST_COMPONENTS "$CACHE{TEST_NAME}" CACHE STRING "List of components to test")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
link_libraries("-u _printf_float -u _scanf_float")
add_compile_definitions(-DMBEDTLS_CMAC_C)
project(lorawan_test)