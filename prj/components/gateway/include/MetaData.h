#ifndef META_DATA_H
#define META_DATA

#include "stdint.h"
#include "MacFrame.h"

typedef struct MetaData_struct
{
    int8_t rssi;
    int8_t snr;
    int8_t id;
    Frame* frame;
} MetaData;

char* MetaData_create_json(MetaData* obj);

#endif