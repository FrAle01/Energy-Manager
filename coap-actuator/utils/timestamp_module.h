#ifndef TIMESTAMP_MODULE_H
#define TIMESTAMP_MODULE_H

int compareTS(char*, char*);
const char* recentTS(const char*, const char*, const char*, const char*);
float extractDay(const char*);
float extractMonth(const char*);
float extractHour(const char*);

#endif