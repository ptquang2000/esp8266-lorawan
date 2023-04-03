idf_component_register(SRCS Gateway.c MetaData.c INCLUDE_DIRS "include" REQUIRES sx1278 mqtt nvs_flash json lorawan)

add_compile_definitions(-DWIFI_SSID="$CACHE{WIFI_SSID}" -DWIFI_PASSWORD="$CACHE{WIFI_PASSWORD}" -DBROKER_URL="$CACHE{BROKER_URL}")