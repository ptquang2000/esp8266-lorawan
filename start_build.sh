#!/bin/bash

source setup.sh
export WORKING_DIR="$(dirname "$0")"
export IDF_PY=$IDF_PATH/tools/idf.py

cd $WORKING_DIR/prj
$IDF_PY -p $1 flash monitor