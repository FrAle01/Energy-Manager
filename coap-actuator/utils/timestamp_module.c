#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "timestamp_module.h"

// timestamp format YYYY-MM-DD_HH:MM:SS

int compareTS(const char* ts0, const char* ts1){
    return strcmp(ts0, ts1);
}

void recentTS(char* arg, const char* ts0, const char* ts1, const char* ts2, const char* ts3){
    char mostRecent[20];
    strcpy(mostRecent, ts0);

    if (compareTS(ts1, mostRecent) > 0) {
        strcpy(mostRecent, ts1);
    }
    if (compareTS(ts2, mostRecent) > 0) {
        strcpy(mostRecent, ts2);
    }
    if (compareTS(ts3, mostRecent) > 0) {
        strcpy(mostRecent, ts3);
    }
    strcpy(arg, mostRecent);
}

float extractDay(const char* ts){
    char day[3];
    strncpy(day, ts + 8, 2);
    day[2] = '\0';

    char* endptr;
    float daynum = strtof(day, &endptr);
    if (*endptr == '\0') {
        return daynum;
    } else {
        return -1;
    }

}

float extractMonth(const char* ts){
    char month[3];
    strncpy(month, ts + 5, 2);
    month[2] = '\0';

    char* endptr;
    float monthnum = strtof(month, &endptr);
    if (*endptr == '\0') {
        return monthnum;
    } else {
        return -1;
    }

}
float extractHour(const char* ts){
    char hour[3];
    strncpy(hour, ts + 11, 2);
    hour[2] = '\0';

    char* endptr;
    float hournum = strtof(hour, &endptr);
    if (*endptr == '\0') {
        return hournum;
    } else {
        return -1;
    }
}

