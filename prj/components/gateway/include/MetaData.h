#ifndef META_DATA_H
#define META_DATA

#include "stdint.h"
#include "MacFrame.h"

typedef struct MetaData_struct
{
    int id;
    double rssi;
    double snr;
    uint8_t* frame;

    uint16_t size;
    char* json;
} MetaData;

void MetaData_create_json(MetaData* obj);
void MetaData_free_json(MetaData* obj);

#endif