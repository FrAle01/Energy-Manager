#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "timestamp.h"

const void get_timestamp(char* arg){
    time_t raw;
    struct tm*  ts;

    time(&raw);
    ts = localtime(&raw);
    // format YYYY-MM-DD_HH:MM:SS
    sprintf(arg, "%d-%02d-%02d_%02d:%02d:%02d", (*ts).tm_year+1900, (*ts).tm_mon+1, (*ts).tm_mday, (*ts).tm_hour, (*ts).tm_min, (*ts).tm_sec);
}