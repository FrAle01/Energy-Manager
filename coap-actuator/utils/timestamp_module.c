#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "timestamp_module.h"

// timestamp format YYYY-MM-DD_HH:MM:SS

int compareTS(char* ts0, char* ts1){
    return strcmp(ts0, ts1);
}

const char* recentTS(const char* ts0, const char* ts1, const char* ts2, const char* ts3){
    const char *mostRecent = ts0;

    if (compareTimestamps(ts1, mostRecent) > 0) {
        mostRecent = ts1;
    }
    if (compareTimestamps(ts2, mostRecent) > 0) {
        mostRecent = ts2;
    }
    if (compareTimestamps(ts3, mostRecent) > 0) {
        mostRecent = ts3;
    }

    return mostRecent;
}

float extractDay(const char* ts){
    char day[3] = NULL;
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
    char month[3] = NULL;
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
    char hour[3] = NULL;
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

