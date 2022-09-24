#include <chrono>

#include "wifi_manager.h"

using namespace std::chrono;

WifiManager::WifiManager()
{
  if (destinationSsid != "" && destinationPassword != "") {
    connectToDestination();
  } else {
    enableSetupServer();
  }
}

void WifiManager::setDestinationConnectionParameters(const char* ssid, const char* password)
{
  destinationSsid = ssid;
  destinationPassword = password;
}

bool WifiManager::connectToDestination()
{
  disableSetupServer();
  milliseconds connectionTimeout = 5s;
  milliseconds waitBetweenAttempts = 250ms;

  WiFi.begin(destinationSsid, destinationPassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(waitBetweenAttempts.count());
    connectionTimeout -= waitBetweenAttempts;

    if (connectionTimeout == 0ms) {
      Serial.println("WiFiManager: Destination connection timeout");
      return false;
    }
  }

  return true;
}

bool WifiManager::enableSetupServer()
{
  WiFi.softAP(setupSsid, setupPassword);
  server.begin();
}