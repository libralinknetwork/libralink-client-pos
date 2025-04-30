#pragma once
#include <Arduino.h>

inline String generateRandomString() {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    size_t charsetSize = sizeof(charset) - 1;
    String id = "";
    for (int i = 0; i < 6; i++) {
        id += charset[random(charsetSize)];
    }
    return id;
}