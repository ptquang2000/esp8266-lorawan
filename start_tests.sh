#!/bin/bash

export WORKING_DIR="$(dirname "$0")"
source setup.sh
export IDF_PY=$IDF_PATH/tools/idf.py
cd $WORKING_DIR/prj/test
$IDF_PY flash monitor
