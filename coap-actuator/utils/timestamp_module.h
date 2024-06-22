#ifndef TIMESTAMP_MODULE_H
#define TIMESTAMP_MODULE_H

int compareTS(const char*, const char*);
void recentTS(char*, const char*, const char*, const char*, const char*);
float extractDay(const char*);
float extractMonth(const char*);
float extractHour(const char*);

#endif