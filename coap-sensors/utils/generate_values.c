#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "generate_values.h"

#ifndef M_PI
    #define M_PI 3.141592653589793
#endif

const float get_rand_value(float mean, float std_dev){
    srand(time(NULL));

    float u1 = (float)rand() / RAND_MAX;
    if(u1 < 1e-100) u1 = 1e-100;
    float u2 = (float)rand() / RAND_MAX;
    
    float z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    return (float)z0 * std_dev + mean;

}

const float generate_curr_battery_cap(float mean, float std_dev){
    float generated_value = get_rand_value(mean, std_dev);

    if(generated_value < 0){
        generated_value = 0;
    }
    if(generated_value > 100){
        generated_value = 100;
    }
    return generated_value;
}