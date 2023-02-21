#!/bin/bash

source setup.sh
export IDF_PY=$IDF_PATH/tools/idf.py
$IDF_PY -C prj -p $1 flash monitor