/*
 * test_publisher.c
 * Connects to the public test broker and publishes 5 simulated sensor readings.
 * Usage: ./test_publisher
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <mosquitto.h>
#include <stdbool.h>

#define BROKER      "127.0.0.1"
#define PORT        1883
#define TOPIC       "mqtt-lab/test/sensor"
#define MSG_COUNT   500
#define INTERVAL_S  2
#define JSON_SIZE 100 // 100 Zeichen zum testen - nicht berechnet

// struct für Sensordaten
typedef struct {
    char timestamp[20]; // 19 Zeichen + \0
    char station_id[4];
    float temperature_c;
    float humidity_pct;
} SensorData;

// Bsp. Sensordaten
SensorData example_data01 = {"2026-05-28 09:09:30", "S02", 24.4f, 74.5f};

// String für JSON Output
static int valid_timestamp(const char *timestamp);
static int validate_sensor_data(const SensorData *in_data);
static int data_to_json(const SensorData *in_data, char *out_buffer, size_t buffer_size);

static int generate_sensor_data(SensorData *data, int seq) {
    time_t now;
    struct tm *t;

    if (data == NULL || seq < 0) {
        return -1;
    }

    now = time(NULL);
    t = gmtime(&now);
    if (t == NULL) {
        return -1;
    }

    if (strftime(data->timestamp, sizeof(data->timestamp), "%Y-%m-%d %H:%M:%S", t) == 0) {
        return -1;
    }

    snprintf(data->station_id, sizeof(data->station_id), "S02");
    data->temperature_c = 18.0f + (float)(rand() % 100) / 20.0f;
    data->humidity_pct = 50.0f + (float)(rand() % 300) / 10.0f;

    (void)seq;
    return 0;
}

static int valid_timestamp(const char *timestamp) {
    int year, month, day, hour, minute, second;
    static const int days_per_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (timestamp == NULL || strlen(timestamp) != 19) {
        return -1;
    }

    if (timestamp[4] != '-' || timestamp[7] != '-' || timestamp[10] != ' ' ||
        timestamp[13] != ':' || timestamp[16] != ':') {
        return -1;
    }

    if (sscanf(timestamp, "%4d-%2d-%2d %2d:%2d:%2d",
               &year, &month, &day, &hour, &minute, &second) != 6) {
        return -1;
    }

    if (year < 1970 || month < 1 || month > 12 || day < 1 || hour < 0 || hour > 23 ||
        minute < 0 || minute > 59 || second < 0 || second > 59) {
        return -1;
    }

    if (day > days_per_month[month - 1]) {
        if (month == 2) {
            bool leap_year = ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
            if (!(leap_year && day == 29)) {
                return -1;
            }
        } else {
            return -1;
        }
    }

    return 0;
}

static int validate_sensor_data(const SensorData *in_data) {
    if (in_data == NULL) {
        return -1;
    }

    if (in_data->timestamp[0] != '\0' && valid_timestamp(in_data->timestamp) != 0) {
        return -1;
    }

    if (in_data->station_id[0] != '\0' && strlen(in_data->station_id) != 3) {
        return -1;
    }

    return 0;
}

static int data_to_json(const SensorData *in_data, char *out_buffer, size_t buffer_size) {
    int written;
    const char *timestamp_value;
    const char *station_id_value;

    if (validate_sensor_data(in_data) != 0 || out_buffer == NULL || buffer_size < JSON_SIZE) {
        return -1;
    }

    timestamp_value = (in_data->timestamp[0] == '\0') ? "empty" : in_data->timestamp;
    station_id_value = (in_data->station_id[0] == '\0') ? "empty" : in_data->station_id;

    written = snprintf(
        out_buffer,
        buffer_size,
        "{\"timestamp\":\"%s\","
        "\"station_id\":\"%s\","
        "\"temperature_c\":%.1f,"
        "\"humidity_pct\":%.1f}",
        timestamp_value,
        station_id_value,
        in_data->temperature_c,
        in_data->humidity_pct);

    if (written < 0 || (size_t)written >= buffer_size) {
        return -1;
    }

    return 0;
}

/* Callback: called when connection is established */
static void on_connect(struct mosquitto *mosq, void *userdata, int rc) {
    (void)userdata;

    if (rc == 0) {
        printf("[publisher] Connected to %s:%d\n", BROKER, PORT);
    } else {
        fprintf(stderr, "[publisher] Connection failed: %s\n",
                mosquitto_connack_string(rc));
        mosquitto_disconnect(mosq);
    }
}

/* Callback: called after each message is published */
static void on_publish(struct mosquitto *mosq, void *userdata, int mid) {
    (void)mosq;
    (void)userdata;

    printf("[publisher] Message %d delivered to broker\n", mid);
}

int main(void) {
    srand((unsigned int)time(NULL));

    mosquitto_lib_init();

    struct mosquitto *mosq = mosquitto_new("mqtt-lab-publisher", true, NULL);
    if (!mosq) {
        fprintf(stderr, "[publisher] Failed to create mosquitto instance\n");
        mosquitto_lib_cleanup();
        return 1;
    }

    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_publish_callback_set(mosq, on_publish);

    int rc = mosquitto_connect(mosq, BROKER, PORT, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "[publisher] Could not connect: %s\n",
                mosquitto_strerror(rc));
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }

    /* Start the network loop in background thread */
    mosquitto_loop_start(mosq);

    SensorData sensor_data;
    char payload[JSON_SIZE];
    printf("[publisher] Sending %d messages to topic: %s\n\n", MSG_COUNT, TOPIC);

    for (int i = 1; i <= MSG_COUNT; i++) {
        if (generate_sensor_data(&sensor_data, i) != 0) {
            fprintf(stderr, "[publisher] Failed to generate sensor data\n");
            continue;
        }

        if (data_to_json(&sensor_data, payload, sizeof(payload)) != 0) {
            fprintf(stderr, "[publisher] Failed to serialize sensor data\n");
            continue;
        }

        printf("[publisher] Publishing: %s\n", payload);

        rc = mosquitto_publish(mosq, NULL, TOPIC,
                               (int)strlen(payload), payload, 1, false);
        if (rc != MOSQ_ERR_SUCCESS) {
            fprintf(stderr, "[publisher] Publish error: %s\n",
                    mosquitto_strerror(rc));
        }
        sleep(INTERVAL_S);
    }

    printf("\n[publisher] Done. Disconnecting.\n");
    mosquitto_disconnect(mosq);
    mosquitto_loop_stop(mosq, true);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
