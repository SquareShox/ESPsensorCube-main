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
void startConfigurationAP();
void stopConfigurationAP();
void WiFiReconnectTask(void *parameter);

// Time helper functions
String getFormattedTime();
String getFormattedDate();
time_t getEpochTime();
bool isTimeSet();
void forceTimeSync();


// Global objects
extern AsyncWebServer server;
extern AsyncEventSource events;

// Global variables
extern unsigned long lastConnectionTime;
extern bool autoReset;

#endif 