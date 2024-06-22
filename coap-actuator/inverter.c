#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "sys/etimer.h"
#include "leds.h"

#include "utils/my_json.h"
#include "utils/queue_manager.h"
#include "utils/timestamp_module.h"

#include "model/IoTmodel.h"


#if PLATFORM_SUPPORTS_BUTTON_HAL
#include "dev/button-hal.h"
#else
#include "dev/button-sensor.h"
#endif

/* Log configuration */

#include "coap-log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL  LOG_LEVEL_DBG


#define SERVER_EP "coap://[fd00::1]:5683"

#define REGISTRATION_ATTEMPTS 5
#define REGISTRATION_DELAY 10 // in seconds

static char* service_url_temp = "/temperature";
static char* service_url_irr = "/irradiance";
static char* service_url_cap = "/capacity";
static char* service_url_cons = "/consumption";

static char* service_url_reg = "/registrationActuator";

// extern coap_resource_t res_batterylimit;

static char temp_addr[50];
static char irr_addr[50];
static char cap_addr[50];
static char cons_addr[50];

static int registration_attempts = 0;
static int registered = 0;

float battery_limit=100; // default battery limit to 100%
#define MAX_CAPACITY 12

PROCESS(coap_client_process, "CoAP Client Process");
AUTOSTART_PROCESSES(&coap_client_process);

static coap_endpoint_t server_ep_temp;
static coap_endpoint_t server_ep_irr;
static coap_endpoint_t server_ep_cap;
static coap_endpoint_t server_ep_cons;


static coap_observee_t *obs_temp = NULL;
static coap_observee_t *obs_irr = NULL;
static coap_observee_t *obs_cap = NULL;
static coap_observee_t *obs_cons = NULL;

static Queue temp_queue;
static Queue irr_queue;
static Queue cap_queue;
static Queue cons_queue;

static float energy_to_house;
static float energy_to_battery;
static float energy_to_sell;



void save_value(char *sensor, double value, char* timestamp){

    if(strcmp(sensor, "temperature") == 0){
        addToQueue(&temp_queue, value, timestamp);

    }else if (strcmp(sensor, "irradiance") == 0){
        addToQueue(&irr_queue, value, timestamp);

    }else if (strcmp(sensor, "irradiance") == 0){
        addToQueue(&cap_queue, value, timestamp);

    }else if (strcmp(sensor, "irradiance") == 0){
        addToQueue(&cons_queue, value, timestamp);

    }
    
}

void handle_notification(struct coap_observee_s *observee, void *notification, coap_notification_flag_t flag) {
    int len = 0;
    const uint8_t *payload = NULL;

    if (notification){
      len = coap_get_payload(notification, &payload);
    }
    switch (flag){

    case NOTIFICATION_OK:
        LOG_INFO("NOTIFICATION OK: %*s\n", len, (char *)payload);

        char *sensor = json_parse_string((char *)payload, "sensor");
        double value = json_parse_number((char *)payload, "value");
        char *timestamp = json_parse_string((char *)payload, "ts");


      // verify if the values are valid: sesnor must not be null and value must be greater than 0
        if (sensor == NULL || value < 0 || timestamp == NULL){
          LOG_INFO("Invalid sensor, value or timestamp\n");
          return;
        }

        // call a function to handle the store of the value
        save_value(sensor, value, timestamp);
        break;

    case OBSERVE_OK:
        LOG_INFO("OBSERVE_OK: %*s\n", len, (char *)payload);
        break;
    case OBSERVE_NOT_SUPPORTED:
        LOG_INFO("OBSERVE_NOT_SUPPORTED: %*s\n", len, (char *)payload);
        obs_temp = NULL;
        obs_irr  = NULL;
        obs_cap  = NULL;
        obs_cons  = NULL;
        break;
    case ERROR_RESPONSE_CODE:
        LOG_INFO("ERROR_RESPONSE_CODE: %*s\n", len, (char *)payload);
        obs_temp = NULL;
        obs_irr  = NULL;
        obs_cap  = NULL;
        obs_cons  = NULL;        break;
    case NO_REPLY_FROM_SERVER:
        LOG_INFO("NO_REPLY_FROM_SERVER: "
                 "removing observe registration");
        obs_temp = NULL;
        obs_irr  = NULL;
        obs_cap  = NULL;
        obs_cons  = NULL;
        break;
    }
}


void registration_handler(coap_message_t *response) {
    if (response == NULL) {
        printf("No response received.\n");
        return;
    }
    const uint8_t *chunk;
    int len = coap_get_payload(response, &chunk);
    char payload[len + 1];
    strncpy(payload, (char *)chunk, len);
    payload[len] = '\0';
    printf("Registration payload: %s\n", payload);

    // Assume payload contains IPv6 addresses in JSON format
    

    char *temp_ip = json_parse_string(payload, "temperature");
    char *irr_ip = json_parse_string(payload, "irradiance");
    char *cap_ip = json_parse_string(payload, "capacity");
    char *cons_ip = json_parse_string(payload, "consumption");



    if (temp_ip != NULL && irr_ip != NULL && cap_ip != NULL && cons_ip != NULL) {
        /*
        char full_ipv6temp[50];
        char full_ipv6lpg[50];
        snprintf(full_ipv6temp, sizeof(full_ipv6temp), "fd00:0:0:0:%s", ipv6temp_item->valuestring);
ù
        strncpy(ipv6temp, full_ipv6temp, sizeof(ipv6temp) - 1);
        ipv6temp[sizeof(ipv6temp) - 1] = '\0';

        strncpy(ipv6lpg, full_ipv6lpg, sizeof(ipv6lpg) - 1);
        ipv6lpg[sizeof(ipv6lpg) - 1] = '\0';

        printf("Registered IPv6temp: %s\n", temp_addr);
        */

        registered = 1;
    } else {
        printf("Invalid JSON format or missing keys\n");
    }
 
}

PROCESS_THREAD(coap_client_process, ev, data) {
    PROCESS_BEGIN();
    static coap_endpoint_t server_ep;
    static coap_message_t request[1];
    static struct etimer registration_timer;
    static struct etimer actuator_timer;
    
    int pressed=0;

    coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);

    while(ev != button_hal_press_event || pressed==0) {
        pressed=1;
        PROCESS_YIELD();
    

        while (registration_attempts < REGISTRATION_ATTEMPTS && !registered) {
            leds_on(LEDS_RED);
            leds_single_on(LEDS_YELLOW);

            coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
            coap_set_header_uri_path(request, service_url_reg);
            coap_set_payload(request, (uint8_t *)"inverter", strlen("inverter"));

            printf("Sending registration request...\n");
            COAP_BLOCKING_REQUEST(&server_ep, request, registration_handler);

            if (!registered) {
                registration_attempts++;
                etimer_set(&registration_timer, CLOCK_SECOND * REGISTRATION_DELAY);
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&registration_timer));
            }
        }

        if (registered) {

            leds_off(LEDS_RED);
            leds_on(LEDS_GREEN);
            leds_single_off(LEDS_YELLOW);

            char uri_temp[100];
            char uri_irr[100];
            char uri_cap[100];
            char uri_cons[100];

            snprintf(uri_temp, sizeof(uri_temp), "coap://[%s]:5683", temp_addr);
            snprintf(uri_irr, sizeof(uri_irr), "coap://[%s]:5683", irr_addr);
            snprintf(uri_cap, sizeof(uri_cap), "coap://[%s]:5683", cap_addr);
            snprintf(uri_cons, sizeof(uri_cons), "coap://[%s]:5683", cons_addr);

            coap_endpoint_parse(uri_temp, strlen(uri_temp), &server_ep_temp);
            coap_endpoint_parse(uri_irr, strlen(uri_irr), &server_ep_irr);
            coap_endpoint_parse(uri_cap, strlen(uri_cap), &server_ep_cap);
            coap_endpoint_parse(uri_cons, strlen(uri_cons), &server_ep_cons);

            printf("Sending observation request to %s\n", uri_temp);
            obs_temp =coap_obs_request_registration(&server_ep_temp, service_url_temp, handle_notification, NULL);

            printf("Sending observation request to %s\n", uri_irr);
            obs_irr =coap_obs_request_registration(&server_ep_irr, service_url_irr, handle_notification, NULL);

            printf("Sending observation request to %s\n", uri_cap);
            obs_cap =coap_obs_request_registration(&server_ep_cap, service_url_cap, handle_notification, NULL);

            printf("Sending observation request to %s\n", uri_cons);
            obs_cons =coap_obs_request_registration(&server_ep_cons, service_url_cons, handle_notification, NULL);


            //coap_activate_resource(&res_batterylimit, "batterylimit");

            etimer_set(&actuator_timer, CLOCK_SECOND * 2);

            initQueue(&temp_queue);
            initQueue(&irr_queue);
            initQueue(&cap_queue);
            initQueue(&cons_queue);

            while (1) {
                PROCESS_WAIT_EVENT();

                if (ev == PROCESS_EVENT_TIMER && etimer_expired(&actuator_timer)) {
                    process_poll(&coap_client_process);

                    if(fullQueue(&temp_queue) && fullQueue(&irr_queue) && fullQueue(&cap_queue) && fullQueue(&cons_queue)){

                        float temp_value = getWMean(&temp_queue, 0.8);
                        float irr_value = getWMean(&irr_queue, 0.8);

                        float battery_cap = getHead(&cap_queue);
                        float house_consumption = getHead(&cons_queue);


                        char newest_ts[20];
                        getRecentTS(newest_ts, &temp_queue, &irr_queue, &cap_queue, &cons_queue);

                        float day = extractDay(newest_ts);
                        float month = extractMonth(newest_ts);
                        float hour = extractHour(newest_ts);

                        float features[5] = {irr_value, temp_value, hour, day, month};

                        float panel_production = IoTmodel_regress1(features, 5);

                        energy_to_house = 0;
                        energy_to_battery = 0;
                        energy_to_sell = 0;

                        if (panel_production > house_consumption){

                            energy_to_house = house_consumption;
                            panel_production -= house_consumption;

                            if(battery_cap >= battery_limit){

                                energy_to_battery = 0;
                                energy_to_sell = panel_production; 

                            }else{

                                energy_to_battery = (battery_limit-battery_cap)*(MAX_CAPACITY/100);
                                if(panel_production > energy_to_battery){
                                    panel_production -= energy_to_battery;
                                    energy_to_sell = panel_production;
                                    panel_production = 0;
                                }else{
                                    energy_to_battery = panel_production;
                                    panel_production = 0;
                                    energy_to_sell = 0;
                                }

                            }

                        }else{
                            energy_to_house = panel_production;
                            panel_production = 0;
                            energy_to_battery = 0;
                            energy_to_sell = 0;
                        }

                    }

                    etimer_reset(&actuator_timer);


                }
            }

        } else {
            printf("Failed to register after %d attempts\n", REGISTRATION_ATTEMPTS);
        }
    }
    PROCESS_END();
}