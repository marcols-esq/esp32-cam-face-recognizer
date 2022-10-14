#include <chrono>
#include <EEPROM.h>

#include "wifi_manager.h"

using namespace std::chrono;
using namespace std;

WifiManager::WifiManager()
: m_isSetupServerEnabled(false)
{
  EEPROM.begin(96);

  m_destinationSsid.reserve(MaxSsidLength);
  m_destinationPassword.reserve(MaxPasswordLength);
  
  for(int i = 0; i < MaxSsidLength; i++) {
    m_destinationSsid[i] = EEPROM.read(i);
    if (m_destinationSsid[i] == 0) {
      break;
    }
  }

  for(int i = 0; i < MaxPasswordLength; i++) {
    m_destinationPassword[i] = EEPROM.read(i + MaxSsidLength);
    if (m_destinationPassword[i] == 0) {
      break;
    }
//    Serial.printf("%d %c\n", i+MaxSsidLength, EEPROM.read(i + MaxSsidLength));
  }
}

void WifiManager::setDestinationConnectionParameters(const string ssid, const string password)
{
  m_destinationSsid = ssid;
  m_destinationPassword = password;
}

bool WifiManager::connectToDestination()
{
  Serial.printf("WifiManager: Trying to connect to %s\n", m_destinationSsid.c_str());
  milliseconds connectionTimeout {5000};
  milliseconds waitBetweenAttempts {250};

  WiFi.begin(m_destinationSsid.c_str(), m_destinationPassword.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(waitBetweenAttempts.count());
    connectionTimeout -= waitBetweenAttempts;

    if (connectionTimeout == milliseconds(0)) {
      Serial.println("WifiManager: Destination connection timeout");
      return false;
    }
  }
  Serial.print("WifiManager: Connected to destination, ESP camera IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

void WifiManager::enableSetupServer()
{
  WiFi.softAP(SetupSsid.c_str(), SetupPassword.c_str());
  Serial.print("WifiManager: Enabling setup server, IP address: ");
  Serial.println(WiFi.softAPIP());
  setupWebPage();
  m_webServer.begin();
  m_isSetupServerEnabled = true;
}

void WifiManager::runSetupServer()
{
  milliseconds time = milliseconds(millis());
  seconds heartbeatCycle = seconds(10);
  uint32_t heartbeatCounter = 0;
  
  while (1) {
    if (!m_isSetupServerEnabled) {
      enableSetupServer();
    }

    m_webServer.handleClient();
    if (milliseconds(millis()) - time > heartbeatCycle) {
      Serial.printf("WifiManager: Setup server heartbeat %d \n", heartbeatCounter++);
      time = milliseconds(millis());
    }
  }
}

void WifiManager::setupWebPage()
{
  m_webServer.on("/", [this]() {
    m_webPage = "<!DOCTYPE HTML><html>";
    m_webPage += "<head><style>";
    m_webPage += "body {background-color: darkgray;}";
    m_webPage += "h1   {font-family: sans-serif; text-align: center;}";
    m_webPage += "p    {font-family: sans-serif; text-align: center;}";
    m_webPage += "</style></head>";
    m_webPage += "<h1>ESP-CAM Face recognizer setup page</h1><br>";
    m_webPage += "<form method='post' action='setting'><label><p>Server SSID:     </label><input name='ssid' length=32></p>";
    m_webPage += "<p>Server password: <input name='pass' length=64 input type='password'> </p>";
    m_webPage += "<br><p>ESP camera IP address: <input name='oct1' length=3 input type='number'>.<input name='oct2' length=3 input type='number'>.";
    m_webPage += "<input name='oct3' length=3 input type='number'>.<input name='oct4' length=3 input type='number'></p>";
    m_webPage += "<p><button>Save</button></p></form>";
    m_webPage += "</html>";
    m_webServer.sendHeader("Connection", "close");
    m_webServer.sendHeader("Access-Control-Allow-Credentials", "true");
    m_webServer.send(200, "text/html", m_webPage);
  });

  m_webServer.on("/setting", [this]() {
    m_webPage = "<!DOCTYPE HTML><html>";
    m_webPage += "<head><style>";
    m_webPage += "body {background-color: darkgray;}";
    m_webPage += "h1   {font-family: sans-serif; text-align: center;}";
    m_webPage += "p    {font-family: sans-serif; text-align: center;}";
    m_webPage += "</style></head>";
    m_webPage += "<h1>ESP-CAM Face recognizer setup page</h1><br>";
    
    string newSsid(m_webServer.arg("ssid").c_str());
    string newPass(m_webServer.arg("pass").c_str());

    if (m_webServer.arg("oct1").toInt() > 255 || m_webServer.arg("oct2").toInt() > 255
        || m_webServer.arg("oct3").toInt() > 255 || m_webServer.arg("oct4").toInt() > 255) {
      m_webPage += "<form method='post' action='setting'><label><p>Server SSID:     </label><input name='ssid' length=32></p>";
      m_webPage += "<p>Server password: <input name='pass' length=64 input type='password'> </p>";
      m_webPage += "<br><p>ESP camera IP address: <input name='oct1' length=3 input type='number'>.<input name='oct2' length=3 input type='number'>.";
      m_webPage += "<input name='oct3' length=3 input type='number'>.<input name='oct4' length=3 input type='number'></p>";
      m_webPage += "<p><button>Save</button></p></form>";
      m_webPage += "<br><p>Invalid IP address, retry</p>";
      m_webServer.sendHeader("Access-Control-Allow-Credentials", "true");
      m_webServer.send(200, "text/html", m_webPage);
      return;
    }
    
    IPAddress newIp(m_webServer.arg("oct1").toInt(), m_webServer.arg("oct2").toInt(), m_webServer.arg("oct3").toInt(), m_webServer.arg("oct4").toInt());

    if (newSsid.length() == 0 || newPass.length() == 0) {
      m_webPage += "<form method='post' action='setting'><label><p>Server SSID:     </label><input name='ssid' length=32></p>";
      m_webPage += "<p>Server password: <input name='pass' length=64 input type='password'> </p>";
      m_webPage += "<br><p>ESP camera IP address: <input name='oct1' length=3 input type='number'>.<input name='oct2' length=3 input type='number'>.";
      m_webPage += "<input name='oct3' length=3 input type='number'>.<input name='oct4' length=3 input type='number'></p>";
      m_webPage += "<p><button>Save</button></p></form>";
      m_webPage += "<br><p>Empty credentials, retry.</p>";
      m_webServer.sendHeader("Access-Control-Allow-Credentials", "true");
      m_webServer.send(200, "text/html", m_webPage);
      return;
    }

    setDestinationConnectionParameters(newSsid, newPass);
    if (connectToDestination()) {
      updateCredentials(newSsid, newPass);
      WiFi.config(newIp, WiFi.gatewayIP(), WiFi.subnetMask());
      m_webPage += "<p>Credentials saved. Rebooting</p></html>";
      m_webServer.sendHeader("Access-Control-Allow-Credentials", "true");
      m_webServer.send(200, "text/html", m_webPage);
      delay(100);
      ESP.restart();
    } else {
      m_webPage += "<form method='post' action='setting'><label><p>Server SSID:     </label><input name='ssid' length=32></p>";
      m_webPage += "<p>Server password: <input name='pass' length=64 input type='password'> </p>";
      m_webPage += "<br><p>ESP camera IP address: <input name='oct1' length=3 input type='number'>.<input name='oct2' length=3 input type='number'>.";
      m_webPage += "<input name='oct3' length=3 input type='number'>.<input name='oct4' length=3 input type='number'></p>";
      m_webPage += "<p><button>Save</button></p></form>";
      m_webPage += "<br><p>Could not connect to the server with following credentials, retry.</p>";
    }
    
    m_webPage += "</html>";
    m_webServer.sendHeader("Access-Control-Allow-Credentials", "true");
    m_webServer.send(200, "text/html", m_webPage);
  });
}

static void writeToEeprom(uint8_t address, uint8_t data) 
{
  if (EEPROM.read(address) != data) {
    EEPROM.write(address, data);
  }
}

void WifiManager::updateCredentials(const string newSsid, const string newPass)
{
  if (m_destinationSsid.length() > newSsid.length()) {
    // clear old ssid characters
    for (int i = (newSsid.length() - 1); i < m_destinationSsid.length(); i++) {
      writeToEeprom(i, 0);
    }
  }

  if (m_destinationPassword.length() > newPass.length()) {
    // clear old password characters
    for (int i = (MaxSsidLength + newPass.length() - 1); i < (MaxSsidLength + m_destinationPassword.length()); i++) {
      writeToEeprom(i, 0);
    }
  }

  // update new credentials
  for (int i = 0; i < newSsid.length(); i++) {
    writeToEeprom(i, newSsid[i]);
  }

  for (int i = 0; i < newPass.length(); i++) {
    writeToEeprom(i + MaxSsidLength, newPass[i]);
  }

  EEPROM.commit();
}
