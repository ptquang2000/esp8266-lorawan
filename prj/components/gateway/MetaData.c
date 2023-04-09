#include "MetaData.h"
#include "cJSON.h"

void MetaData_create_json(MetaData* obj)
{
    char *string = NULL;

    cJSON *metaData = cJSON_CreateObject();
    if (metaData == NULL)
    {
        goto end;
    }
    if (cJSON_AddNumberToObject(metaData, "id", obj->id) == NULL)
    {
        goto end;
    }
    if (cJSON_AddNumberToObject(metaData, "rssi", obj->rssi) == NULL)
    {
        goto end;
    }
    if (cJSON_AddNumberToObject(metaData, "snr", obj->snr) == NULL)
    {
        goto end;
    }
    cJSON* frame = cJSON_AddArrayToObject(metaData, "frame");
    if (frame == NULL)
    {
        goto end;
    }
    for (size_t i = 0; i < obj->size; i++)
    {
        cJSON* number = cJSON_CreateNumber(obj->frame[i]);
        if (number == NULL)
        {
            goto end;
        }
        cJSON_AddItemToArray(frame, number);
    }

    string = cJSON_PrintUnformatted(metaData);
    if (string == NULL)
    {
        fprintf(stderr, "Failed to print meta data.\n");
    }

end:
    cJSON_Delete(metaData);
    obj->json = string;
}

void MetaData_free_json(MetaData* obj)
{
    free(obj->json);
}