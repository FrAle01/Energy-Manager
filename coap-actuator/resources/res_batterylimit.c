#include "contiki.h"
#include "contiki-net.h"
#include "coap-engine.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "coap-log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL  LOG_LEVEL_DBG

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                            uint16_t preferred_size, int32_t *offset);
static void res_post_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                             uint16_t preferred_size, int32_t *offset);

extern float battery_limit;  // Default limit value
static char battery_str[24] = "100.00";

RESOURCE(res_batterylimit,
         "title=\"Set Battery Limit\";rt=\"Text\"",
         res_get_handler,
         res_post_handler,
         NULL,
         NULL);

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                            uint16_t preferred_size, int32_t *offset) {
    printf("GET ricevuta\n");

    snprintf(battery_str, sizeof(battery_str), "%d.%02d", (int)battery_limit, (int)abs(((battery_limit-(int)battery_limit)*100)));
    int length = snprintf((char*)buffer, preferred_size, "{\"limit\":\"%s\"}\n", battery_str);
    if (length > preferred_size) {
        length = preferred_size;
    }
   
    // Set response fields
    coap_set_header_content_format(response, APPLICATION_JSON);
    coap_set_header_etag(response, (uint8_t *)&length, 1);
    coap_set_payload(response, buffer, length);

}

static void res_post_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                             uint16_t preferred_size, int32_t *offset) {
    // Declare a temporary variable to store the new threshold
    printf("POST ricevuta\n");
    
    const uint8_t *payload = NULL;
    int payload_len = coap_get_payload(request, &payload);

    // Check if payload is present and is valid
    if (payload_len) {
        char new_limit_str[16];
        if (payload_len >= sizeof(new_limit_str)) {
            coap_set_status_code(response, BAD_REQUEST_4_00);
            return;
        }

        // Copy the payload into a temporary buffer
        memcpy(new_limit_str, payload, payload_len);
        new_limit_str[payload_len] = '\0';

        // Convert the payload to float
        battery_limit = atof(new_limit_str);

        // Log the new threshold
        LOG_INFO("New battery limit set: %.2f\n", battery_limit);

        // Construct the response payload
        int length = snprintf((char *)buffer, preferred_size, "Threshold set to: \"%s\"", new_limit_str);

        // Set response fields
        coap_set_header_content_format(response, TEXT_PLAIN);
        coap_set_header_etag(response, (uint8_t *)&length, 1);

        // Set response payload
        coap_set_payload(response, (uint8_t *)buffer, length);
    } else {
        // No payload, set response code to BAD_REQUEST
        coap_set_status_code(response, BAD_REQUEST_4_00);
    }
}