#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include "config.h"

// Function declarations
void initializeWiFi();
void initializeWebServer();
void WiFiReconnectTask(void *parameter);

// Global objects
extern AsyncWebServer server;
extern AsyncEventSource events;

// Global variables
extern unsigned long lastConnectionTime;
extern bool autoReset;

#endif 