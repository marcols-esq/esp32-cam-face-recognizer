#include <chrono>

#include "wifi_manager.h"

using namespace std::chrono;
using namespace std;

WifiManager::WifiManager()
: m_isSetupServerEnabled(false)
{
  m_server = WiFiServer(Port);
}

void WifiManager::setDestinationConnectionParameters(const string ssid, const string password)
{
  m_destinationSsid = ssid;
  m_destinationPassword = password;
}

bool WifiManager::connectToDestination()
{
  milliseconds connectionTimeout {5000};
  milliseconds waitBetweenAttempts {250};

  WiFi.begin(m_destinationSsid.c_str(), m_destinationPassword.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(waitBetweenAttempts.count());
    connectionTimeout -= waitBetweenAttempts;

    if (connectionTimeout == milliseconds(0)) {
      Serial.println("WiFiManager: Destination connection timeout");
      return false;
    }
  }

  return true;
}

void WifiManager::enableSetupServer()
{
  WiFi.softAP(SetupSsid.c_str(), SetupPassword.c_str());
  Serial.print("Enabling setup server, IP address: ");
  Serial.println(WiFi.softAPIP());
  m_server.begin();
  m_isSetupServerEnabled = true;
}

bool WifiManager::isClientAvailable()
{
  m_client = m_server.available();
  return m_client;
}

char WifiManager::readClientData()
{
  if (m_client.available()) {
    return m_client.read();
  }
  
  return NULL;
}

void WifiManager::sendDataToClient(const char* msg)
{
  m_client.println("HTTP/1.1 200 OK");
  m_client.println("Content-type:text/html");
  m_client.println();
  
  m_client.print(msg);
}

void WifiManager::runSetupServer()
{
  bool once;
  while (1) {
    if (!m_isSetupServerEnabled) {
      enableSetupServer();
    }
    
    if (isClientAvailable()) {
      while(m_client.connected()) {             
        if (!once) {
          once = true;
          sendDataToClient("hEllo");
        }
      }
    }
  }
}
