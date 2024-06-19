#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "coap-engine.h"
#include "math.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP


static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

EVENT_RESOURCE(res_consumption,
               "title=\"Observable resource\";energymeter",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               res_event_handler);

static double curr_consumption = 0;

static void res_event_handler(void)
{
  curr_consumption = 0;
  LOG_INFO("Payload to be sent: {\"sensor\":\"energymeter\", \"value\":%.2f}\n", curr_consumption);
  coap_notify_observers(&res_consumption);
}

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_set_header_content_format(response, APPLICATION_JSON);
  int payload_len = snprintf((char *)buffer, preferred_size, "{\"sensor\":\"energymeter\", \"value\":%.2f}", curr_consumption);
  coap_set_payload(response, buffer, payload_len);

  LOG_INFO("Payload: %s\n", buffer);
}