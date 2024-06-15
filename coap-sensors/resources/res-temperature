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

EVENT_RESOURCE(res_temperature,
               "title=\"Observable resource\";temperature",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               res_event_handler);

static double curr_temperature = 0;

static void res_event_handler(void)
{
  curr_temperature = 0;
  LOG_INFO("Payload to be sent: {\"sensor\":\"temperature\", \"value\":%.2f}\n", curr_temperature);
  coap_notify_observers(&res_temperature);
}

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_set_header_content_format(response, APPLICATION_JSON);
  int payload_len = snprintf((char *)buffer, preferred_size, "{\"sensor\":\"temperature\", \"value\":%.2f}", curr_temperature);
  coap_set_payload(response, buffer, payload_len);

  LOG_INFO("Payload: %s\n", buffer);
}