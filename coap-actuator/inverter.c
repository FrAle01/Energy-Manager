#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "contiki.h"
#include "contiki-net.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "sys/etimer.h"
#include "leds.h"

#include "utils/my_json.h"
#include "utils/queue_manager.h"

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

#define MAX_ADDR_REQUEST 3  // max attempts for each address request
#define ADDR_REQUEST_DELAY 5
#define MAX_TIME_REQUEST 5
#define MAX_CAPACITY 12

static char* service_url_temp = "/temperature";
static char* service_url_irr = "/irradiance";
static char* service_url_cap = "/capacity";
static char* service_url_cons = "/consumption";

static char* service_url_reg = "/registrationActuator";
static char* service_url_addr_req = "/addressRequest";
static char* service_url_ts_req = "/timeRequest";


extern coap_resource_t res_batterylimit;
extern coap_resource_t res_energyflow;


static char temp_addr[50];
static char irr_addr[50];
static char cap_addr[50];
static char cons_addr[50];

static int temp_addr_ok = 0;
static int irr_addr_ok = 0;
static int cap_addr_ok = 0;
static int cons_addr_ok = 0;

static int registration_attempts = 0;
static int registered = 0;

static int address_received = 0;

float battery_limit = 100; // default battery limit to 100%

PROCESS(coap_client_process, "CoAP Client Process");
AUTOSTART_PROCESSES(&coap_client_process);

static coap_endpoint_t server_ep_temp;
static coap_endpoint_t server_ep_irr;
static coap_endpoint_t server_ep_cap;
static coap_endpoint_t server_ep_cons;

static coap_endpoint_t server_ep;
static coap_message_t request[1];

static coap_observee_t *obs_temp = NULL;
static coap_observee_t *obs_irr = NULL;
static coap_observee_t *obs_cap = NULL;
static coap_observee_t *obs_cons = NULL;

static Queue temp_queue;
static Queue irr_queue;
static Queue cap_queue;
static Queue cons_queue;

static int time_requests = 0;
static int got_time = 0;

static float day = 0;
static float month = 0;
static float hour = 0;

float panel_production = 0;
 
float energy_to_house = 0;
float energy_to_battery = 0;
float energy_to_sell = 0;


void save_value(char *sensor, float value){

    if(strcmp(sensor, "tm") == 0){
        addToQueue(&temp_queue, value);
        LOG_INFO("Received value :%.2f, for sensor: %s \n Queue size:%d", value, sensor, temp_queue.count);

    }else if (strcmp(sensor, "ir") == 0){
        addToQueue(&irr_queue, value);
        LOG_INFO("Received value :%.2f, for sensor: %s \n Queue size:%d", value, sensor, irr_queue.count);

    }else if (strcmp(sensor, "cp") == 0){
        addToQueue(&cap_queue, value);
        LOG_INFO("Received value :%.2f, for sensor: %s \n Queue size:%d", value, sensor, cap_queue.count);

    }else if (strcmp(sensor, "cn") == 0){
        addToQueue(&cons_queue, value);
        LOG_INFO("Received value :%.2f, for sensor: %s \n Queue size:%d", value, sensor, cons_queue.count);

    }else{
        LOG_INFO("No value inserted");
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
        char *str_value = json_parse_string((char *)payload, "value");

        float value = atof(str_value);


      // verify if the values are valid: sesnor must not be null and value must be greater than 0
        if (sensor == NULL || value < 0){
          LOG_INFO("Invalid sensor or value\n");
          return;
        }

        // call a function to handle the store of the value
        save_value(sensor, value);
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

    registered = 1;
 
}

void address_response_handler(coap_message_t *response){
    if (response == NULL) {
        printf("No address response received.\n");
        return;
    }

    const uint8_t *chunk;
    int len = coap_get_payload(response, &chunk);
    char payload[len + 1];
    strncpy(payload, (char *)chunk, len);
    payload[len] = '\0';
    printf("Addressess request payload: %s\n", payload);

    char *sensor = json_parse_string(payload, "sensor");
    char *sensor_ip = json_parse_string(payload, "addr");

    if(sensor_ip != NULL){
        if(strcmp(sensor, "temperature") == 0){
            strcpy(temp_addr, sensor_ip);
            temp_addr_ok = 1;

        }else if(strcmp(sensor, "irradiance") == 0){
            strcpy(irr_addr, sensor_ip);
            irr_addr_ok = 1;

        }else if(strcmp(sensor, "capacity") == 0){
            strcpy(cap_addr, sensor_ip);
            cap_addr_ok = 1;

        }else if(strcmp(sensor, "consumption") == 0){
            strcpy(cons_addr, sensor_ip);
            cons_addr_ok = 1;

        }
        address_received = 1;

    }else{
        address_received = 0;
    }
}

void timestamp_response_handler(coap_message_t *response){
    if (response == NULL) {
        printf("No time values received.\n");
        got_time = 0;
        return;
    }

    const uint8_t *chunk;
    int len = coap_get_payload(response, &chunk);
    char payload[len + 1];
    strncpy(payload, (char *)chunk, len);
    payload[len] = '\0';
    printf("Timestamp request payload: %s\n", payload);

    month = json_parse_number(payload, "month");
    day = json_parse_number(payload, "day");
    hour = json_parse_number(payload, "hour");

    

    time_requests = 0;
    got_time = 1;

}


PROCESS_THREAD(coap_client_process, ev, data) {
    PROCESS_BEGIN();
    static struct etimer registration_timer;
    static struct etimer actuator_timer;
    
    int pressed=0;

    coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);

    while(ev != button_hal_press_event || pressed==0) {
        pressed=1;
        PROCESS_YIELD();
    
        leds_single_on(LEDS_YELLOW);
        while (registration_attempts < REGISTRATION_ATTEMPTS && !registered) {
            leds_on(LEDS_RED);
            leds_single_off(LEDS_YELLOW);

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
            leds_on(LEDS_BLUE);
            leds_single_on(LEDS_YELLOW);

            static struct etimer address_timer;
            //get_sensor_addr("temperature");


            int address_request = 0;
            address_received = 0;

        
            while(address_request < MAX_ADDR_REQUEST && !address_received){

                coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
                coap_set_header_uri_path(request, service_url_addr_req);

                char *buff = "{\"a\": \"inverter\", \"s\": \"temperature\"}";
                coap_set_payload(request, (uint8_t *)buff, strlen(buff));

                printf("Sending temperature address request...\n");
                COAP_BLOCKING_REQUEST(&server_ep, request, address_response_handler);
                if (!address_received) {
                    address_request++;
                    etimer_set(&address_timer, CLOCK_SECOND * ADDR_REQUEST_DELAY);
                    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&address_timer));
                }
            }

            //get_sensor_addr("irradiance");
            address_request = 0;
            address_received = 0;

            while(address_request < MAX_ADDR_REQUEST && !address_received){

                coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
                coap_set_header_uri_path(request, service_url_addr_req);

                char *buff = "{\"a\": \"inverter\", \"s\": \"irradiance\"}";
                coap_set_payload(request, (uint8_t *)buff, strlen(buff));

                printf("Sending irradiance address request...\n");
                COAP_BLOCKING_REQUEST(&server_ep, request, address_response_handler);
                if (!address_received) {
                    address_request++;
                    etimer_set(&address_timer, CLOCK_SECOND * ADDR_REQUEST_DELAY);
                    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&address_timer));
                }
            }
            
            //get_sensor_addr("capacity");
            address_request = 0;
            address_received = 0;

            while(address_request < MAX_ADDR_REQUEST && !address_received){

                coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
                coap_set_header_uri_path(request, service_url_addr_req);

                char *buff = "{\"a\": \"inverter\", \"s\": \"capacity\"}";
                coap_set_payload(request, (uint8_t *)buff, strlen(buff));

                printf("Sending capacity address request...\n");
                COAP_BLOCKING_REQUEST(&server_ep, request, address_response_handler);
                if (!address_received) {
                    address_request++;
                    etimer_set(&address_timer, CLOCK_SECOND * ADDR_REQUEST_DELAY);
                    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&address_timer));
                }
            }

            //get_sensor_addr("consumption");
            address_request = 0;
            address_received = 0;

            while(address_request < MAX_ADDR_REQUEST && !address_received){

                coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
                coap_set_header_uri_path(request, service_url_addr_req);

                char *buff = "{\"a\": \"inverter\", \"s\": \"consumption\"}";
                coap_set_payload(request, (uint8_t *)buff, strlen(buff));

                printf("Sending consumption address request...\n");
                COAP_BLOCKING_REQUEST(&server_ep, request, address_response_handler);
                if (!address_received) {
                    address_request++;
                    etimer_set(&address_timer, CLOCK_SECOND * ADDR_REQUEST_DELAY);
                    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&address_timer));
                }
            }


            if (temp_addr_ok && irr_addr_ok && cap_addr_ok && cons_addr_ok){
                leds_single_off(LEDS_YELLOW);
                leds_off(LEDS_BLUE);
                leds_on(LEDS_GREEN);
            }else{
                leds_off(LEDS_BLUE);
                leds_on(LEDS_RED);
                printf("Failed to receive sensor addesses from server");
                exit(1);
            }
            

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


            coap_activate_resource(&res_batterylimit, "batterylimit");
            coap_activate_resource(&res_energyflow, "energyflow");

            etimer_set(&actuator_timer, CLOCK_SECOND * 10);

            initQueue(&temp_queue);
            initQueue(&irr_queue);
            initQueue(&cap_queue);
            initQueue(&cons_queue);

            while (1) {
                PROCESS_WAIT_EVENT();

                if (ev == PROCESS_EVENT_TIMER && etimer_expired(&actuator_timer)) {
                    process_poll(&coap_client_process);

                    if(fullQueue(&temp_queue) && fullQueue(&irr_queue) && fullQueue(&cap_queue) && fullQueue(&cons_queue)){
                        LOG_INFO("Calculating new energy distribution\n");

                        got_time = 0;

                        while(time_requests < MAX_TIME_REQUEST && !got_time){
                            coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
                            coap_set_header_uri_path(request, service_url_ts_req);

                            char *buff = "time";
                            coap_set_payload(request, (uint8_t *)buff, strlen(buff));

                            printf("Sending time request to server...\n");
                            COAP_BLOCKING_REQUEST(&server_ep, request, timestamp_response_handler);

                            if (!got_time) {
                                time_requests++;
                                etimer_set(&address_timer, CLOCK_SECOND * 1);
                                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&address_timer));
                            }
                        }
                    

                        if(got_time){

                            float temp_value = getWMean(&temp_queue, 0.8);
                            LOG_INFO("Using temp mean: %.2f\n", temp_value);
                            float irr_value = getWMean(&irr_queue, 0.8);
                            LOG_INFO("Using irr mean: %.2f\n", irr_value);
    
                            float battery_cap = getHead(&cap_queue);
                            LOG_INFO("And residual battery: %.2f\n", battery_cap);
                            LOG_INFO("Battery limit setted: %.2f\n", battery_limit);
    
                            float house_consumption = getHead(&cons_queue);
                            LOG_INFO("With house consumption: %.2f\n", house_consumption);

                            LOG_INFO("Data to ml: irr>%d, temp>%d, month>%d, day>%d, hour>%d\n", (int)irr_value, (int)temp_value, (int)month, (int)day, (int)hour);


                            float features[5] = {irr_value, temp_value, hour, day, month};

                            panel_production = IoTmodel_regress1(features, 5);
                            LOG_INFO("Produced: %d\n", (int)panel_production);

                            float residual_energy = panel_production/100;

                            /*
                            if(isnan(panel_production)){
                                residual_energy = 0;
                            }else{
                                residual_energy = panel_production;
                            }
                            */

                            energy_to_house = 0;
                            energy_to_battery = 0;
                            energy_to_sell = 0;

                            if (residual_energy > house_consumption){

                                energy_to_house = house_consumption;
                                residual_energy -= house_consumption;

                                if(battery_cap >= battery_limit){

                                    energy_to_battery = 0;
                                    energy_to_sell = residual_energy;
                                    residual_energy = 0; 

                                }else{

                                    energy_to_battery = (battery_limit-battery_cap)/100;
                                    energy_to_battery *= MAX_CAPACITY;
                                    LOG_INFO("Energy to battery calculated: %.2f\n", energy_to_battery);

                                    if(residual_energy > energy_to_battery){
                                        residual_energy -= energy_to_battery;
                                        energy_to_sell = residual_energy;
                                        residual_energy = 0;
                                    }else{
                                        energy_to_battery = residual_energy;
                                        residual_energy = 0;
                                        energy_to_sell = 0;
                                    }

                                }

                            }else{
                                energy_to_house = residual_energy;
                                residual_energy = 0;
                                energy_to_battery = 0;
                                energy_to_sell = 0;
                            }

                            res_energyflow.trigger();
                        }else{
                            printf("Failed to get timestamp from server\n");

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