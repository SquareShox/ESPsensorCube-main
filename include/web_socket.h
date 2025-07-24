#ifndef WEB_SOCKET_H
#define WEB_SOCKET_H

#include <Arduino.h>
#include <AsyncWebSocket.h>

// Deklaracje funkcji WebSocket
void initializeWebSocket(AsyncWebSocket& ws);
void broadcastSensorData(AsyncWebSocket& ws);

// Nowe funkcje dla zarzadzania pamiecia i natywnego ping/pong
void cleanupWebSocketMemory();
void sendPingToClient(AsyncWebSocketClient* client);
void checkWebSocketConnections();

// Zaawansowane czyszczenie pamieci
void forceGarbageCollection();
void intelligentMemoryCleanup();
void performEmergencyCleanup();
void addWebSocketClient(AsyncWebSocketClient* client);
void removeWebSocketClient(AsyncWebSocketClient* client);
void updateClientPongTime(AsyncWebSocketClient* client);
void sendNativePing(AsyncWebSocketClient* client);
void cleanupInactiveClients();

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

// Zmienne globalne dla zarzadzania pamiecia
extern unsigned long lastPingTime;
extern unsigned long lastCleanupTime;
extern unsigned long lastMemoryCheck;
extern unsigned long lastNativePingTime;
extern int wsClientCount;

#endif // WEB_SOCKET_H 