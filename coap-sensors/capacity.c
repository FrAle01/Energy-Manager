#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "coap-engine.h"
#include "random.h"
#include "contiki-net.h"
#include "sys/log.h"
#include "coap-blocking-api.h"
#include "os/dev/button-hal.h"
#include "leds.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

#define SERVER_EP "coap://[fd00::1]:5683"
#define TIME_SAMPLE 5
#define MAX_REGISTRATION_RETRY 3
#define GOOD_ACK 65
#define MAX_SAMPLES 10

extern coap_resource_t res_capacity;

static int registration_retry_count = 0;
static int registered = 0;
static int shutdown=0;


PROCESS(battery_process, "Battery Process");
AUTOSTART_PROCESSES(&battery_process);

static struct etimer capacity_timer;

void client_chunk_handler(coap_message_t *response) {

    const uint8_t *chunk;
    if (response == NULL) {
        LOG_ERR("Request timed out\n");
        printf("Request timed out\n");
        return;
    }else{
        int len = coap_get_payload(response, &chunk);
        char payload[len + 1];
        memcpy(payload, chunk, len);
        payload[len] = '\0';  // Ensure null-terminated string
        printf("Response: %i\n", response->code);

        if (response->code == GOOD_ACK) {
            printf("Registration successful\n");
            registered = 1;
        } else {
            printf("Registration failed\n");
        }

    }
}

PROCESS_THREAD(battery_process, ev, data){

    //button_hal_button_t *btn;
    static coap_endpoint_t server_ep;
    static coap_message_t request[1];
    // btn= button_hal_get_by_index(0);
    int pressed=0;

    PROCESS_BEGIN();

    random_init(0); // Initialize random number generator
    while(ev != button_hal_press_event || pressed==0) {
        pressed=1;
        PROCESS_YIELD();
    
    coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);
    while (registration_retry_count < MAX_REGISTRATION_RETRY && registered == 0) {
        // Initialize POST request
            leds_on(LEDS_RED);
        leds_single_on(LEDS_YELLOW);

        coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
        coap_set_header_uri_path(request, "/registrationSensor");
        
        const char payload[] = "capacity";
        
        coap_set_payload(request, (uint8_t *)payload, strlen(payload)-1);
        

        printf("Sending the registration request\n");
        // Send the blocking request
        COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);

        if (registered == 0) {
            LOG_INFO("Retry registration (%d/%d)\n", registration_retry_count, MAX_REGISTRATION_RETRY);
            registration_retry_count++;
            etimer_set(&capacity_timer, CLOCK_SECOND * 5); // Wait 5 seconds before retrying
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&capacity_timer));
        }
    }

    if (registered == 1) {
        //shutdown=0;
        leds_off(LEDS_RED);
        leds_on(LEDS_GREEN);
        leds_single_off(LEDS_YELLOW);

        printf("Activate server term\n");
        //LOG_INFO("Starting Erbium Example Server\n");
        
        // Activate resources
        coap_activate_resource(&res_capacity, "capacity");

        printf("CoAP server started\n");

        // Main loop
        etimer_set(&capacity_timer, CLOCK_SECOND * 3);
   

        while (1) {

            PROCESS_YIELD();
            if(shutdown==1){
                printf("Shutdown incremented\n");
            
                process_exit(&battery_process);
                PROCESS_EXIT();

            }
            if (etimer_expired(&capacity_timer)) {
                res_capacity.trigger();
                etimer_reset(&capacity_timer);
            }
            
        }
        // mando notifica a tutti di spegnere
        printf("shutdown  %i\n", shutdown);

        

    } else {
        printf("Reached maximum number of registration attempts\n");
    }

} 


    PROCESS_END();
}