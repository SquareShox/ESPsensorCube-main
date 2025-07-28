#ifndef WEB_SOCKET_H
#define WEB_SOCKET_H

#include <Arduino.h>
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>

// Deklaracje funkcji WebSocket
void initializeWebSocket(AsyncWebSocket& ws);
void broadcastSensorData(AsyncWebSocket& ws);

// WebSocket Task System
bool initializeWebSocketTask();
void stopWebSocketTask();
bool sendToWebSocketTask(AsyncWebSocketClient* client, const String& message);
void webSocketTask(void* parameters);
void handleWebSocketMessageInTask(AsyncWebSocketClient* client, DynamicJsonDocument& doc);

// WebSocket Monitoring i Reset
void checkHeapAndReset();
void checkWebSocketActivity();
void resetWebSocket();
void forceWebSocketReset(const String& reason);
String getWebSocketStatus();
void checkWebSocketClients();

// Funkcje klientow WebSocket
void addWebSocketClient(AsyncWebSocketClient* client);
void removeWebSocketClient(AsyncWebSocketClient* client);
void updateClientPongTime(AsyncWebSocketClient* client);
void sendNativePing(AsyncWebSocketClient* client);

// Główna funkcja obsługi wiadomości WebSocket
void handleWebSocketMessage(AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len);

// Zmienne globalne dla zarzadzania pamiecia
extern unsigned long lastPingTime;
extern unsigned long lastCleanupTime;
extern unsigned long lastMemoryCheck;
extern unsigned long lastNativePingTime;
extern int wsClientCount;

#endif // WEB_SOCKET_H 