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
static char house_str[7] = "0.00";
static char battery_str[7] = "0.00";
static char sell_str[7] = "0.00";

extern char newest_ts[20];
extern char newest_time[9];

EVENT_RESOURCE(res_energyflow,
               "title=\"Observable resource\";energyflow",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               res_event_handler);



static void res_event_handler(void)
{

  snprintf(house_str, 6, "%.2f", energy_to_house);
  snprintf(battery_str, 6, "%.2f", energy_to_battery);
  snprintf(sell_str, 6, "%.2f", energy_to_sell);
  LOG_INFO("Payload to be sent: {\"h\":\"%s\", \"b\":\"%s\", \"g\":\"%s\", \"ts\":\"%s\"}\n", house_str, battery_str, sell_str, newest_time);
  coap_notify_observers(&res_energyflow);
}

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_set_header_content_format(response, APPLICATION_JSON);
  int payload_len = snprintf((char *)buffer, preferred_size, "{\"h\":\"%s\", \"b\":\"%s\", \"g\":\"%s\", \"ts\":\"%s\"}\n", house_str, battery_str, sell_str, newest_time);
  coap_set_payload(response, buffer, payload_len);

  LOG_INFO("Payload: %s\n", buffer);
}