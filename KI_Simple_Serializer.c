#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned int sequence;
    long timestamp;
    int station_id;
    float temperature_c;
    float humidity_pct;
} MeasurementData;

char* serialize_measurement_json(const MeasurementData* data) {
    if (data == NULL) {
        return NULL;
    }

    // Größe grob kalkuliert
    const size_t buffer_size = 256;

    char* json = (char*)malloc(buffer_size);
    if (json == NULL) {
        return NULL;
    }

    snprintf(
        json,
        buffer_size,
        "{"
        "\"sequence\":%u,"
        "\"timestamp\":%ld,"
        "\"station_id\":%d,"
        "\"temperature_c\":%.2f,"
        "\"humidity_pct\":%.2f"
        "}",
        data->sequence,
        data->timestamp,
        data->station_id,
        data->temperature_c,
        data->humidity_pct
    );

    return json;
}

int main() {
    MeasurementData data = {
        .sequence = 42,
        .timestamp = 1748428800,
        .station_id = 7,
        .temperature_c = 23.56f,
        .humidity_pct = 61.25f
    };

    char* json = serialize_measurement_json(&data);

    if (json != NULL) {
        printf("%s\n", json);
        free(json);
    }

    return 0;
}