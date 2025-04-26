#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>

void setupFileManagerEndpoints(AsyncWebServer& server);

bool createFolder(const String& path);
bool createFile(const String& folder, const String& filename, const String& content);
String readFile(const String& path);
bool deletePath(const String& path);
String listFolder(const String& path);