#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "coap-engine.h"
#include <math.h>
#include "../utils/timestamp.h"
#include "../utils/generate_values.h"


#include <locale.h>

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

static float curr_temperature = 0.0f;
static char temp_str[24] = "0.00";

static void res_event_handler(void)
{
  curr_temperature = get_rand_value(15.0, 15.0);
  snprintf(temp_str, sizeof(temp_str), "%d.%02d", (int)curr_temperature, (int)abs(((curr_temperature-(int)curr_temperature)*100)));

  LOG_INFO("New temp int: %d, dec: %02d, in string: %s", (int)curr_temperature, (int)abs(((curr_temperature-(int)curr_temperature)*100)), temp_str);
  LOG_INFO("Payload to be sent: {\"sensor\":\"tm\", \"value\":\"%s\"}\n", temp_str);
  coap_notify_observers(&res_temperature);
}

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  setlocale(LC_NUMERIC, "C");

  coap_set_header_content_format(response, APPLICATION_JSON);
  int payload_len = snprintf((char *)buffer, preferred_size, "{\"sensor\":\"tm\", \"value\":\"%s\"}\n", temp_str);
  coap_set_payload(response, buffer, payload_len);
  LOG_INFO("Preferred size: %d\n", preferred_size);
  LOG_INFO("Payload: %s\n", buffer);
}