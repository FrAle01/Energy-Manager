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

extern float energy_to_house;
extern float energy_to_battery;
extern float energy_to_sell;
extern char newest_ts[20];

EVENT_RESOURCE(res_energyflow,
               "title=\"Observable resource\";energyflow",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               res_event_handler);



static void res_event_handler(void)
{
  LOG_INFO("Payload to be sent: {\"h\":%.2f, \"b\":%.2f, \"g\":%.2f, \"ts\":\"%s\"}\n", energy_to_house, energy_to_battery, energy_to_sell, newest_ts);
  coap_notify_observers(&res_energyflow);
}

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_set_header_content_format(response, APPLICATION_JSON);
  int payload_len = snprintf((char *)buffer, preferred_size, "{\"h\":%.2f, \"b\":%.2f, \"g\":%.2f, \"ts\":\"%s\"}\n", energy_to_house, energy_to_battery, energy_to_sell, newest_ts);
  coap_set_payload(response, buffer, payload_len);

  LOG_INFO("Payload: %s\n", buffer);
}