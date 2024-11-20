#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bigIntUtil.h"

char *getCurrentDateTime() {
    char *dateTimeString = (char *) malloc(20 * sizeof(char)); // "DD-MM-YYYY HH:MM:SS\0"
    mallocCheck(dateTimeString);

    // Get the current time
    time_t currentTime;
    time(&currentTime);

    // Convert to local time format
    struct tm *localTime = localtime(&currentTime);

    snprintf(dateTimeString, 20, "%02d-%02d-%04d %02d:%02d:%02d",
             localTime->tm_mday,
             localTime->tm_mon + 1,       // tm_mon is 0-based
             localTime->tm_year + 1900,   // tm_year is years since 1900
             localTime->tm_hour,
             localTime->tm_min,
             localTime->tm_sec);

    return dateTimeString;
}