#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "generate_values.h"

#ifndef M_PI
    #define M_PI 3.141592653589793
#endif

void rand_init(){
    srand(1234);

}

double get_rand_value(double mean, double std_dev){

    double u1 = (double)rand() / RAND_MAX;
    if(u1 < 1e-100) u1 = 1e-100;
    double u2 = (double)rand() / RAND_MAX;
    
    double z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    return (double)z0 * std_dev + mean;

}

double generate_curr_battery_cap(double mean, double std_dev){
    double generated_value = get_rand_value(mean, std_dev);

    if(generated_value < 0){
        generated_value = 0;
    }
    if(generated_value > 100){
        generated_value = 100;
    }
    return generated_value;
}