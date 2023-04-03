#include "MetaData.h"
#include "cJSON.h"

char* MetaData_create_json(MetaData* obj)
{
    char *string = NULL;
    cJSON *GatewayID = NULL;
    cJSON *Rssi = NULL;
    cJSON *Snr = NULL;
    cJSON *Frame = NULL;

    cJSON *metaData = cJSON_CreateObject();
    if (metaData == NULL)
    {
        goto end;
    }

    GatewayID = cJSON_CreateNumber(obj->id);
    if (GatewayID == NULL)
    {
        goto end;
    }
    cJSON_AddItemToObject(metaData, "GatewayID", GatewayID);

    Rssi = cJSON_CreateNumber(obj->rssi);
    if (Rssi == NULL)
    {
        goto end;
    }
    cJSON_AddItemToObject(metaData, "Rssi", Rssi);

    Snr = cJSON_CreateNumber(obj->snr);
    if (Snr == NULL)
    {
        goto end;
    }
    cJSON_AddItemToObject(metaData, "Snr", Snr);

    Frame = cJSON_CreateString((char*) obj->frame->data);
    if (Snr == NULL)
    {
        goto end;
    }
    cJSON_AddItemToObject(metaData, "Frame", Frame);

    string = cJSON_Print(metaData);
    if (string == NULL)
    {
        fprintf(stderr, "Failed to print meta data.\n");
    }

end:
    cJSON_Delete(metaData);
    return string;
}