#ifndef GATEWAY_H
#define GATEWAY_H

#include "stdint.h"
#include "string.h"

typedef struct SX1278_struct SX1278;

void Gateway_register_event();
void Gateway_initialize();
SX1278* Gateway_get_lora();

#endif