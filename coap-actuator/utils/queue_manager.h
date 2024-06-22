#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

#define QUEUE_SIZE 12
#define TS_SIZE 20

typedef struct{
    double values[QUEUE_SIZE];
    char ts[QUEUE_SIZE][TS_SIZE];
    int head;
    int count;

} Queue;

void initQueue(Queue*);
void addToQueue(Queue*, double, char*);
int fullQueue(Queue*);
float getWMean(Queue*, float);
float getHead(Queue*);
void getRecentTS(char*, Queue*, Queue*, Queue*, Queue*);


#endif