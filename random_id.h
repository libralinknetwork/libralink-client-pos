#pragma once
#include <Arduino.h>

inline const char* generateRandomId() {
    static char id[7];
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    size_t charsetSize = sizeof(charset) - 1;
    for (int i = 0; i < 6; i++) {
        id[i] = charset[random(charsetSize)];
    }
    id[6] = '\0';
    return id;
}