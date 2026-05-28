#include <stdio.h>
#include <string.h>

typedef struct {
    int seq;
    char station_id[16];
    char timestamp[32];
    float temperature_c;
    float humidity_pct;
} SensorReading;

/*
 * serialize_to_json()
 *
 * Regeln laut MQTT-Labor:
 * - Rückgabe: 0 bei Erfolg, -1 bei Fehler
 * - buf darf nicht NULL sein
 * - r darf nicht NULL sein
 * - Mindestgröße Buffer: 80 Bytes
 * - Feldnamen exakt:
 *   seq
 *   station_id
 *   timestamp
 *   temperature_c
 *   humidity_pct
 * - Zahlen mit 1 Nachkommastelle
 * - Kein trailing newline
 */

int serialize_to_json(
    const SensorReading *r,
    char *buf,
    size_t buf_len
) {
    int written;

    if (r == NULL || buf == NULL || buf_len < 80) {
        return -1;
    }

    written = snprintf(
        buf,
        buf_len,
        "{\"seq\":%d,"
        "\"station_id\":\"%s\","
        "\"timestamp\":\"%s\","
        "\"temperature_c\":%.1f,"
        "\"humidity_pct\":%.1f}",
        r->seq,
        r->station_id,
        r->timestamp,
        r->temperature_c,
        r->humidity_pct
    );

    /* snprintf Fehler oder Buffer zu klein */
    if (written < 0 || (size_t)written >= buf_len) {
        return -1;
    }

    return 0;
}

/* Test */
int main(void) {
    SensorReading reading = {
        .seq = 1,
        .station_id = "S1",
        .timestamp = "2024-03-01T08:15:32Z",
        .temperature_c = 19.3f,
        .humidity_pct = 64.2f
    };

    char json_buf[256];

    if (serialize_to_json(&reading, json_buf, sizeof(json_buf)) == 0) {
        printf("%s\n", json_buf);
    } else {
        printf("Serializer Fehler\n");
    }

    return 0;
}