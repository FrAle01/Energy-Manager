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
static char house_str[24] = "0.00";
static char battery_str[24] = "0.00";
static char sell_str[24] = "0.00";


EVENT_RESOURCE(res_energyflow,
               "title=\"Observable resource\";energyflow",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               res_event_handler);



static void res_event_handler(void)
{

  snprintf(house_str, sizeof(house_str), "%d.%02d", (int)energy_to_house, (int)abs(((energy_to_house-(int)energy_to_house)*100)));
  snprintf(battery_str, sizeof(battery_str), "%d.%02d", (int)energy_to_battery, (int)abs(((energy_to_battery-(int)energy_to_battery)*100)));
  snprintf(sell_str, sizeof(sell_str), "%d.%02d", (int)energy_to_sell, (int)abs(((energy_to_sell-(int)energy_to_sell)*100)));
  LOG_INFO("Payload to be sent: {\"h\":\"%s\", \"b\":\"%s\", \"g\":\"%s\"}\n", house_str, battery_str, sell_str);
  coap_notify_observers(&res_energyflow);
}

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  coap_set_header_content_format(response, APPLICATION_JSON);
  int payload_len = snprintf((char *)buffer, preferred_size, "{\"h\":\"%s\", \"b\":\"%s\", \"g\":\"%s\"}\n", house_str, battery_str, sell_str);
  coap_set_payload(response, buffer, payload_len);

  LOG_INFO("Payload: %s\n", buffer);
}