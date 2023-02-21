#!/bin/bash

source setup.sh
export IDF_PY=$IDF_PATH/tools/idf.py
$IDF_PY -C prj fullclean
$IDF_PY -C prj/test fullclean
