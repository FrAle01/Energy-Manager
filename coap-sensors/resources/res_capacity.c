#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "coap-engine.h"
#include "math.h"
#include "utils/timestamp.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

#define MAX_CAPACITY 12 // kWh

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

EVENT_RESOURCE(res_capacity,
               "title=\"Observable resource\";capacity",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               res_event_handler);

static double curr_perc_capacity = 0; 

static void res_event_handler(void)
{
  curr_perc_capacity = 0; // random % between 0 and 100
  LOG_INFO("Payload to be sent: {\"sensor\":\"capacity\", \"value\":%.2f, \"ts\":%s}\n", curr_perc_capacity, get_timestamp());
  coap_notify_observers(&res_capacity);
}

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_set_header_content_format(response, APPLICATION_JSON);
  int payload_len = snprintf((char *)buffer, preferred_size, "{\"sensor\":\"capacity\", \"value\":%.2f}", curr_perc_capacity);
  coap_set_payload(response, buffer, payload_len);

  LOG_INFO("Payload: %s\n", buffer);
}