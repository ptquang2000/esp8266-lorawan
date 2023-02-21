#!/bin/bash

if [ $1 == "-b" ] && [ -n "$2" ]
then ESPPORT="$2" 
fi
source setup.sh
export IDF_PY=$IDF_PATH/tools/idf.py
$IDF_PY -C prj/test -p $ESPPORT flash monitor