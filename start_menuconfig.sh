#!/bin/bash

source setup.sh
export IDF_PY=$IDF_PATH/tools/idf.py
cd prj
$IDF_PY menuconfig