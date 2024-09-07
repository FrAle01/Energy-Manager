#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "queue_manager.h"

void initQueue(Queue* q){
    q->head = -1;
    q->count = 0;
}

void addToQueue(Queue* q, double value){

    

    q->head = (q->head +1) % QUEUE_SIZE;
    q->values[q->head] = value;
    
    if(q->count < QUEUE_SIZE){
        q->count++;
    }

}

float getWMean(Queue* q, float weight){ // calculates the mean of the values in the queue, giving more weight to the newest value
    
    float mean = 0;

    if(weight == 0){    // if weight == 0 the mean is not weighted

        float sum = 0;
        for (int i = 0; i < QUEUE_SIZE; i++){

            sum += (q->values[i]);

        }
        mean = sum/QUEUE_SIZE;

    }else{

        mean = weight*(q->values[q->head]);
        float res_weight = (1-weight)/(QUEUE_SIZE-1);

        for (int i = 0; i < QUEUE_SIZE; i++){

            if(i != q->head){
                mean += res_weight*(q->values[i]);
            }
        }
    }

    return mean;
    

}

float getHead(Queue* q){
    return q->values[q->head];
}


int fullQueue(Queue* q){
    return q->count == QUEUE_SIZE;
}