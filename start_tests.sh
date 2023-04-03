#!/bin/bash

source setup.sh
export IDF_PY=$IDF_PATH/tools/idf.py
$IDF_PY -C prj/test "$@" flash monitor