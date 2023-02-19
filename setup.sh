#! /bin/bash

export PATH="$PATH:$(pwd)/xtensa-lx106-elf/bin"
export IDF_PATH="$(pwd)/ESP8266_RTOS_SDK"

chmod +x start_clean.sh start_tests.sh start_menuconfig.sh
alias clean=./start_clean.sh
alias test=./start_tests.sh
alias menuconfig=./start_menuconfig.sh
alias build=./start_build.sh
alias idf=$IDF_PATH/tools/idf.py