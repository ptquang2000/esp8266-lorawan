#! /bin/bash

export PATH="$PATH:$(pwd)/xtensa-lx106-elf/bin"
export IDF_PATH="$(pwd)/ESP8266_RTOS_SDK"
export IDF_PY=$IDF_PATH/tools/idf.py
export PRJ_DIR=./prj
export TEST_DIR=./prj/test

test_func() {
    local component=$1
    local port=$2
    shift 2
    $IDF_PY -C $TEST_DIR -p $port -D TEST_COMPONENTS="$component" "$@" flash monitor
}

run_func() {
    local port=$1
    shift 1
    $IDF_PY -C $PRJ_DIR -p $port $@ flash monitor
}

alias clean="$IDF_PY -C $PRJ_DIR fullclean && $IDF_PY -C $TEST_DIR fullclean"
alias menuconfig="$IDF_PY -C $PRJ_DIR menuconfig"
alias build="$IDF_PY -C $PRJ_DIR all"
alias run=run_func
alias test=test_func
alias idf="$IDF_PY"
