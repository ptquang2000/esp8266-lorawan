#! /bin/bash

export PATH="$PATH:$(pwd)/xtensa-lx106-elf/bin"
export IDF_PATH="$(pwd)/ESP8266_RTOS_SDK"
export ESPPORT=COM10

chmod +x run_clean.sh run_tests.sh
