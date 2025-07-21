#ifndef WEB_SOCKET_H
#define WEB_SOCKET_H

#include <Arduino.h>
#include <AsyncWebSocket.h>

// Deklaracje funkcji WebSocket
void initializeWebSocket(AsyncWebSocket& ws);
void broadcastSensorData(AsyncWebSocket& ws);

// Deklaracje funkcji obsługi komend
void handleGetStatus(AsyncWebSocketClient* client, void* arg);
void handleGetSensorData(AsyncWebSocketClient* client, void* arg);
void handleGetHistory(AsyncWebSocketClient* client, void* arg);
void handleGetHistoryInfo(AsyncWebSocketClient* client, void* arg);
void handleGetAverages(AsyncWebSocketClient* client, void* arg);
void handleSetConfig(AsyncWebSocketClient* client, void* arg);
void handleGetConfig(AsyncWebSocketClient* client, void* arg);
void handleSystemCommand(AsyncWebSocketClient* client, void* arg);
void handleCalibrationCommand(AsyncWebSocketClient* client, void* arg);

// Główna funkcja obsługi wiadomości WebSocket
void handleWebSocketMessage(AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len);

#endif // WEB_SOCKET_H 