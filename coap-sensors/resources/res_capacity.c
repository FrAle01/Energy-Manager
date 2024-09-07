#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "coap-engine.h"
#include "../utils/generate_values.h"


#include <locale.h>


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

static float curr_perc_capacity = 0.0f; 
static char cap_str[24] = "0.00";

static void res_event_handler(void)
{
  curr_perc_capacity = generate_curr_battery_cap(70.0, 30.0); // random % between 0 and 100
  snprintf(cap_str, sizeof(cap_str), "%d.%02d", (int)curr_perc_capacity, (int)abs(((curr_perc_capacity-(int)curr_perc_capacity)*100)));


  LOG_INFO("Payload to be sent: {\"sensor\":\"cp\", \"value\":\"%s\"}\n", cap_str);
  coap_notify_observers(&res_capacity);
}

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  setlocale(LC_NUMERIC, "C");

  coap_set_header_content_format(response, APPLICATION_JSON);
  int payload_len = snprintf((char *)buffer, preferred_size, "{\"sensor\":\"cp\", \"value\":\"%s\"}\n", cap_str);
  coap_set_payload(response, buffer, payload_len);

  LOG_INFO("Payload: %s\n", buffer);
}