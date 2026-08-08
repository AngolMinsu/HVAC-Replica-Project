#ifndef HEAD_UNIT_WIFI_MANAGER_H
#define HEAD_UNIT_WIFI_MANAGER_H

#include <stddef.h>
#include <stdint.h>

enum HeadUnitWifiState : uint8_t {
  HEAD_UNIT_WIFI_OFF = 0,
  HEAD_UNIT_WIFI_IDLE,
  HEAD_UNIT_WIFI_SCANNING,
  HEAD_UNIT_WIFI_CONNECTING,
  HEAD_UNIT_WIFI_CONNECTED,
  HEAD_UNIT_WIFI_FAILED,
};

struct HeadUnitWifiNetwork {
  char ssid[33];
  int32_t rssi;
  bool secured;
};

void wifiManagerBegin();
void wifiManagerTick();

bool wifiManagerRequestEnabled(bool enabled);
bool wifiManagerRequestScan();
bool wifiManagerRequestConnect(const char* ssid, const char* password);
bool wifiManagerRequestDisconnect();

HeadUnitWifiState wifiManagerGetState();
uint32_t wifiManagerGetRevision();
uint32_t wifiManagerGetNetworkRevision();
size_t wifiManagerGetNetworkCount();
bool wifiManagerGetNetwork(size_t index, HeadUnitWifiNetwork* network);
void wifiManagerGetConnectedSsid(char* destination, size_t destinationSize);

#endif
