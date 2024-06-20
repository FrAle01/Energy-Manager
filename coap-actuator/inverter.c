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

extern coap_resource_t res_batterytresh;

static char temp_addr[50];
static char irr_addr[50];
static char cap_addr[50];
static char cons_addr[50];

static int registration_attempts = 0;
static int registered = 0;

float battery_limit=100; // default battery limit to 100%

PROCESS(coap_client_process, "CoAP Client Process");
AUTOSTART_PROCESSES(&coap_client_process);


static coap_observee_t *obs_temp = NULL;
static coap_observee_t *obs_irr = NULL;
static coap_observee_t *obs_cap = NULL;
static coap_observee_t *obs_cons = NULL;



void response_handler_IRR(coap_message_t *response) {
    if(response==NULL) {
        printf("No response received.\n");
        return;
    }

    const uint8_t *chunk;
    int len = coap_get_payload(response, &chunk);
    printf("|%.*s\n", len, (char *)chunk);
   

    int value = atoi((char *)chunk);
    lpgValue = value;
    printf("lpg valore: %d \n",value);
    if(lpgValue == 1) {
        printf("lpg is normal\n");
        leds_on(LEDS_GREEN);
    } else if(lpgValue == 2) {
        printf("lpg is dangerous\n");
        leds_on(LEDS_RED);
    } else if(lpgValue == 3) {
        nRisklpg++;
        printf("lpg is critical\n");
        leds_on(LEDS_RED);
    } else {
        printf("lpg is unknown\n");
    }
}

void response_handler_TEMP(coap_message_t *response) {
    printf("response_handler\n");
    if(response == NULL) {
        printf("No response received.\n");
        return;
    }
    const uint8_t *chunk;
    int len = coap_get_payload(response, &chunk);
    char temp_str[len + 1];
    strncpy(temp_str, (char *)chunk, len);
    temp_str[len] = '\0';
    tempValue = atof(temp_str);
   // printf("Temperature Response: |%s| (Parsed: %.2f)\n", temp_str, tempValue);
}

void handle_notification(struct coap_observee_s *observee, void *notification, coap_notification_flag_t flag) {
    coap_message_t *msg = (coap_message_t *)notification;
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
        if (sensor == NULL || value < 0 || timestamp == NULL)
        {
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
        obs = NULL;
        break;
    case ERROR_RESPONSE_CODE:
        LOG_INFO("ERROR_RESPONSE_CODE: %*s\n", len, (char *)payload);
        obs = NULL;
        break;
    case NO_REPLY_FROM_SERVER:
        LOG_INFO("NO_REPLY_FROM_SERVER: "
                 "removing observe registration with token %x%x\n",
                 obs->token[0], obs->token[1]);
        obs = NULL;
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
    

    char *temp_ip = json_parse_string(payload, "temp");
    char *irr_ip = json_parse_string(payload, "irr");
    char *cap_ip = json_parse_string(payload, "cap");
    char *cons_ip = json_parse_string(payload, "cons");



    if (temp_ip != NULL && irr_ip != NULL && cap_ip != NULL && cons_ip != NULL) {
        char full_ipv6temp[50];
        char full_ipv6lpg[50];
        snprintf(full_ipv6temp, sizeof(full_ipv6temp), "fd00:0:0:0:%s", ipv6temp_item->valuestring);
ù
        strncpy(ipv6temp, full_ipv6temp, sizeof(ipv6temp) - 1);
        ipv6temp[sizeof(ipv6temp) - 1] = '\0';

        strncpy(ipv6lpg, full_ipv6lpg, sizeof(ipv6lpg) - 1);
        ipv6lpg[sizeof(ipv6lpg) - 1] = '\0';

        printf("Registered IPv6temp: %s\n", temp_addr);

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

    coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);

    while (registration_attempts < REGISTRATION_ATTEMPTS && !registered) {
        coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
        coap_set_header_uri_path(request, service_url_reg);
        coap_set_payload(request, (uint8_t *)"actuator", strlen("actuator"));

        printf("Sending registration request...\n");
        COAP_BLOCKING_REQUEST(&server_ep, request, registration_handler);

        if (!registered) {
            registration_attempts++;
            etimer_set(&registration_timer, CLOCK_SECOND * REGISTRATION_DELAY);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&registration_timer));
        }
    }

    if (registered) {

        


        static coap_endpoint_t server_ep_temp;
        static coap_endpoint_t server_ep_irr;
        static coap_endpoint_t server_ep_cap;
        static coap_endpoint_t server_ep_cons;


        char uri_temp[100];
        char uri_irr[100];
        char uri_cap[100];
        char uri_cons[100];

        snprintf(uri_temp, sizeof(uri_temp), "coap://[%s]:5683", temp_addr);
        snprintf(uri_irr, sizeof(uri_irr), "coap://[%s]:5683", irr_addr);
        snprintf(uri_cap, sizeof(uri_cap), "coap://[%s]:5683", cap_addr);
        snprintf(uri_cons, sizeof(uri_cons), "coap://[%s]:5683", cons_addr);


        coap_endpoint_parse(uri_temp, strlen(uri_temp), &server_ep_temp);
        coap_endpoint_parse(uri_lpg, strlen(uri_lpg), &server_ep_lpg);

        printf("Sending observation request to %s\n", uri_temp);
        obs_temp =coap_obs_request_registration(&server_ep_temp, service_url_temp, handle_notification, NULL);

        printf("Sending observation request to %s\n", uri_irr);
        obs_irr =coap_obs_request_registration(&server_ep_irr, service_url_irr, handle_notification, NULL);

        printf("Sending observation request to %s\n", uri_cap);
        obs_cap =coap_obs_request_registration(&server_ep_cap, service_url_cap, handle_notification, NULL);

        printf("Sending observation request to %s\n", uri_cons);
        obs_cons =coap_obs_request_registration(&server_ep_cons, service_url_cons, handle_notification, NULL);


        coap_activate_resource(&res_batterylimit, "batterylimit");

        etimer_set(&main_timer, CLOCK_SECOND * 2);
        //shutdown=0;
        while (1) {
            PROCESS_WAIT_EVENT();

            if(temp_tresh==-1 || shutdown==1){ // set a shutdown event
                shutdown=1;
                printf("Shutdown incremented\n");

                if (obs_temp != NULL) {
                   coap_obs_remove_observee(obs_temp);
                }
                if (obs_irr != NULL) {
                   coap_obs_remove_observee(obs_irr);
                }
                if (obs_cap != NULL) {
                   coap_obs_remove_observee(obs_cap);
                }
                if (obs_cons != NULL) {
                   coap_obs_remove_observee(obs_cons);
                }
                
                
                process_exit(&coap_client_process);
                PROCESS_EXIT();

            }
            if (ev == PROCESS_EVENT_TIMER && etimer_expired(&main_timer)) {
                process_poll(&coap_client_process);
                etimer_reset(&main_timer);

            }
        }
       
    } else {
        printf("Failed to register after %d attempts\n", REGISTRATION_ATTEMPTS);
    }

    PROCESS_END();
}