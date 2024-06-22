#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "queue_manager.h"
#include "timestamp_module.h"

void initQueue(Queue* q){
    q->head = -1;
    q->count = 0;
}

void addToQueue(Queue* q, double value, char* ts){

    if(q->count == QUEUE_SIZE){
        q->count = 0;
    }

    q->head = (q->head +1) % QUEUE_SIZE;
    q->values[q->head] = value;
    strncpy(q->ts[q->head], ts, TS_SIZE - 1);
    q->ts[q->head][TS_SIZE - 1] = '\0';
    q->count++;


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

void getRecentTS(char* arg, Queue* q0, Queue* q1, Queue* q2, Queue* q3){

    recentTS(arg, q0->ts[q0->head], q1->ts[q1->head], q2->ts[q2->head], q3->ts[q3->head]);

}


int fullQueue(Queue* q){
    return q->count == QUEUE_SIZE;
}