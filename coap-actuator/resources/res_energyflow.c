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

extern float panel_production;

extern float energy_to_house;
extern float energy_to_battery;
extern float energy_to_sell;

EVENT_RESOURCE(res_energyflow,
               "title=\"Observable resource\";energyflow",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               res_event_handler);



static void res_event_handler(void)
{
  LOG_INFO("Payload to be sent: {\"produced\":\"%.2f\", \"home\":%.2f, \"battery\":\"%.2f, \"sold\":\"%.2f\"}\n", panel_production, energy_to_house, energy_to_battery, energy_to_sell);
  coap_notify_observers(&res_energyflow);
}

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_set_header_content_format(response, APPLICATION_JSON);
  int payload_len = snprintf((char *)buffer, preferred_size, "{\"produced\":\"%.2f\", \"home\":%.2f, \"battery\":\"%.2f, \"sold\":\"%.2f\"}\n", panel_production, energy_to_house, energy_to_battery, energy_to_sell);
  coap_set_payload(response, buffer, payload_len);

  LOG_INFO("Payload: %s\n", buffer);
}