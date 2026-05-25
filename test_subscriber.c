/*
 * test_subscriber.c
 * Connects to the public test broker and prints every message on the lab topic.
 * Usage: ./test_subscriber   (keep running while publisher sends)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>

#define BROKER  "127.0.0.1"
#define PORT    1883
#define TOPIC   "mqtt-lab/test/sensor"

static int msg_count = 0;
static int* msg_count_ptr = &msg_count;

/* Callback: called when connection is established */
static void on_connect(struct mosquitto *mosq, void *userdata, int rc) {
    if (rc == 0) {
        printf("[subscriber] Connected to %s:%d\n", BROKER, PORT);
        printf("[subscriber] Subscribing to: %s\n\n", TOPIC);

    } else {
        fprintf(stderr, "[subscriber] Connection failed: %s\n",
                mosquitto_connack_string(rc));
    }

}

/* Callback called when the broker sends a SUBACK in response to a SUBSCRIBE. */
static void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	int i;
	bool have_subscription = false;

	/* In this example we only subscribe to a single topic at once, but a
	 * SUBSCRIBE can contain many topics at once, so this is one way to check
	 * them all. */
	for(i=0; i<qos_count; i++){
		printf("on_subscribe: %d:granted qos = %d\n", i, granted_qos[i]);
		if(granted_qos[i] <= 2){
			have_subscription = true;
		}
	}
	if(have_subscription == false){
		/* The broker rejected all of our subscriptions, we know we only sent
		 * the one SUBSCRIBE, so there is no point remaining connected. */
		fprintf(stderr, "Error: All subscriptions rejected.\n");
		mosquitto_disconnect(mosq);
	}
}

/* Callback: called when a message arrives */
static void on_message(struct mosquitto *mosq, void *userdata,
                       const struct mosquitto_message *msg) {
    msg_count++;
    printf("[subscriber] Message #%d received:\n", msg_count);
    printf("  Topic:   %s\n", msg->topic);
    printf("  Payload: %.*s\n\n", msg->payloadlen, (char *)msg->payload);
}

/* Callback: called on disconnect */
static void on_disconnect(struct mosquitto *mosq, void *userdata, int rc) {
    if (rc != 0) {
        printf("[subscriber] Unexpected disconnect (rc=%d) – will reconnect\n", rc);
    } else {
        printf("[subscriber] Disconnected cleanly. Received %d messages total.\n",
               msg_count);
    }
}

int main(void) {
    mosquitto_lib_init();

    struct mosquitto *mosq = mosquitto_new("mqtt-lab-subscriber2313123", true, NULL);
    if (!mosq) {
        fprintf(stderr, "[subscriber] Failed to create mosquitto instance\n");
        mosquitto_lib_cleanup();
        return 1;
    }

    mosquitto_connect_callback_set(mosq, on_connect);
    printf("[subscriber] after mosquitto_connect_callback_set(mosq, on_connect); \n");
    mosquitto_subscribe_callback_set(mosq, on_subscribe);
    mosquitto_message_callback_set(mosq, on_message);
    printf("[subscriber] after mosquitto_message_callback_set(mosq, on_message); \n");
    mosquitto_disconnect_callback_set(mosq, on_disconnect);
    printf("[subscriber] after mosquitto_disconnect_callback_set(mosq, on_disconnect); \n");
    

    int rc = mosquitto_connect(mosq, BROKER, PORT, 0);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "[subscriber] Could not connect: %s\n",
                mosquitto_strerror(rc));
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }
    
    rc = mosquitto_subscribe(mosq, NULL, TOPIC, 0);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
		/* We might as well disconnect if we were unable to subscribe */
		mosquitto_disconnect(mosq);
	}
    printf("[subscriber] Waiting for messages. Press Ctrl+C to stop.\n\n");

    /* Loop forever – handles reconnects automatically */
    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
