#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "coap-engine.h"
#include <math.h>
#include "../utils/timestamp.h"
#include "../utils/generate_values.h"


/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

#include <locale.h>

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

EVENT_RESOURCE(res_irradiance,
               "title=\"Observable resource\";irradiance",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               res_event_handler);

static double curr_irradiance = 0;
static char ts[20];

static void res_event_handler(void)
{
  curr_irradiance = fmax(get_rand_value(500.0, 250.0), 0.0);
  get_timestamp(ts);
  
  LOG_INFO("Payload to be sent: {\"sensor\":\"ir\", \"value\":%.2f, \"ts\":\"%s\"}\n", curr_irradiance, ts);
  coap_notify_observers(&res_irradiance);
}

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  setlocale(LC_NUMERIC, "C");

  coap_set_header_content_format(response, APPLICATION_JSON);
  int payload_len = snprintf((char *)buffer, preferred_size, "{\"sensor\":\"ir\", \"value\":%.2f, \"ts\":\"%s\"}\n", curr_irradiance, ts);
  coap_set_payload(response, buffer, payload_len);

  LOG_INFO("Payload: %s\n", buffer);
}