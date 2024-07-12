#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "my_json.h"

char* json_parse_string(const char* json, const char* key) {
    const char* key_start = strstr(json, key);
    if (key_start == NULL) {
        return NULL;
    }
    key_start += strlen(key) + 3;
    const char* value_end = strchr(key_start, '"');
    if (value_end == NULL) {
        return NULL;
    }

    size_t value_length = value_end - key_start;
    char* value = (char*)malloc(value_length + 1);
    if (value == NULL) {
        return NULL;
    }

    strncpy(value, key_start, value_length);
    value[value_length] = '\0';
    return value;
}

double json_parse_number(const char* json, const char* key) {
    const char* key_start = strstr(json, key);
    if (key_start == NULL) {
        return -1;
    }
    key_start += strlen(key) + 2;
    double value;
    sscanf(key_start, "%lf", &value);
    return value;
}

char* json_parse_object(const char* json, const char* key) {
    const char* key_start = strstr(json, key);
    if (key_start == NULL) {
        return NULL;
    }
    key_start += strlen(key) + 2;
    const char* value_end = key_start;
    int brace_count = 1;
    while (brace_count > 0) {
        if (*value_end == '{') {
            brace_count++;
        } else if (*value_end == '}') {
            brace_count--;
        }
        value_end++;
    }

    size_t value_length = value_end - key_start;
    char* value = (char*)malloc(value_length + 1);
    if (value == NULL) {
        return NULL;
    }

    strncpy(value, key_start, value_length);
    value[value_length] = '\0';
    return value;
}
