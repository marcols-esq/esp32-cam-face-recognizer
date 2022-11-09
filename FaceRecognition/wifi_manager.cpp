#include <chrono>
#include "config.h"
#include <EEPROM.h>

#include "wifi_manager.h"

using namespace std::chrono;
using namespace std;

WifiManager::WifiManager()
: m_isSetupServerEnabled(false)
{
  EEPROM.begin(wifi_MaxSsidLength + wifi_MaxPasswordLength + mqtt_MaxMqttHostLength + mqtt_MaxMqttPasswordLength + mqtt_MaxPortLength);

  m_destinationSsid.reserve(wifi_MaxSsidLength);
  m_destinationPassword.reserve(wifi_MaxPasswordLength);
  
  for(int i = 0; i < wifi_MaxSsidLength; i++) {
    m_destinationSsid += EEPROM.read(i);
//    Serial.println(m_destinationSsid[i]);
    if (m_destinationSsid[i] == 0) {
      break;
    }
  }

  for(int i = 0; i < wifi_MaxPasswordLength; i++) {
    m_destinationPassword += EEPROM.read(i + wifi_MaxSsidLength);
//    Serial.println(m_destinationPassword[i]);
    if (m_destinationPassword[i] == 0) {
      break;
    }
  }
}

void WifiManager::setDestinationConnectionParameters(const string ssid, const string password)
{
  m_destinationSsid = ssid;
  m_destinationPassword = password;
}

bool WifiManager::connectToDestination(const std::string ssid, const std::string password)
{
  milliseconds connectionTimeout {10000};
  milliseconds waitBetweenAttempts {250};
  // if arguments are default
//  Serial.println(ssid.empty());
//  Serial.println(ssid.length());
//  Serial.println(m_destinationSsid.c_str());
  if (ssid.empty()) {
    Serial.printf("WifiManager: Trying to connect to %s\n", m_destinationSsid.c_str());
    WiFi.begin(m_destinationSsid.c_str(), m_destinationPassword.c_str());
  } else {
    Serial.printf("WifiManager: Trying to connect to %s\n", ssid.c_str());
    WiFi.begin(ssid.c_str(), password.c_str());
  }

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
    m_webPage += "<form method='post' action='setting'><label><p>Server SSID:     </label><input name='ssid' length=" + String(wifi_MaxSsidLength) + "></p>";
    m_webPage += "<p>Server password: <input name='pass' length=" + String(wifi_MaxPasswordLength) + "input type='password'> </p>";
    m_webPage += "<br><p>ESP camera IP address: <input name='oct1' length=3 input type='number' style='width: 50px'>.";
    m_webPage += "<input name='oct2' length=3 input type='number' style='width: 50px'>.<input name='oct3' length=3 input type='number' style='width: 50px'>.";
    m_webPage += "<input name='oct4' length=3 input type='number' style='width: 50px'></p>";
    m_webPage += "<p>MQTT Server username: " + String(mqtt_Username.c_str()) + "</p>";
    m_webPage += "<p>MQTT Server host address: <input name='mqtt_host' length=" + String(mqtt_MaxMqttHostLength) + "> Port: <input name='mqtt_port' length=5 style='width: 70px'></p>";
    m_webPage += "<p>MQTT Server password: <input name='mqtt_pass' length=" + String(mqtt_MaxMqttPasswordLength) + " input type='password'></p>";
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
    string newMqttHost(m_webServer.arg("mqtt_host").c_str());
    string newMqttPass(m_webServer.arg("mqtt_pass").c_str());
    string newMqttPort(m_webServer.arg("mqtt_port").c_str());

    if (m_webServer.arg("oct1").toInt() > 255 || m_webServer.arg("oct2").toInt() > 255
        || m_webServer.arg("oct3").toInt() > 255 || m_webServer.arg("oct4").toInt() > 255) {
      m_webPage += "<form method='post' action='setting'><label><p>Server SSID:     </label><input name='ssid' length=" + String(wifi_MaxSsidLength) + "></p>";
      m_webPage += "<p>Server password: <input name='pass' length=" + String(wifi_MaxPasswordLength) + "input type='password'> </p>";
      m_webPage += "<br><p>ESP camera IP address: <input name='oct1' length=3 input type='number' style='width: 50px'>.";
      m_webPage += "<input name='oct2' length=3 input type='number' style='width: 50px'>.<input name='oct3' length=3 input type='number' style='width: 50px'>.";
      m_webPage += "<input name='oct4' length=3 input type='number' style='width: 50px'></p>";
      m_webPage += "<p>MQTT Server username: " + String(mqtt_Username.c_str()) + "</p>";
      m_webPage += "<p>MQTT Server host address: <input name='mqtt_host' length=" + String(mqtt_MaxMqttHostLength) + "> Port: <input name='mqtt_port' length=5 style='width: 70px'></p>";
      m_webPage += "<p>MQTT Server password: <input name='mqtt_pass' length=" + String(mqtt_MaxMqttPasswordLength) + " input type='password'></p>";
      m_webPage += "<p><button>Save</button></p></form>";
      m_webPage += "<br><p>Invalid ESP camera IP address, retry</p>";
      m_webServer.sendHeader("Access-Control-Allow-Credentials", "true");
      m_webServer.send(200, "text/html", m_webPage);
      return;
    }
    
    IPAddress newIp(m_webServer.arg("oct1").toInt(), m_webServer.arg("oct2").toInt(), m_webServer.arg("oct3").toInt(), m_webServer.arg("oct4").toInt());

    if (newSsid.length() == 0 || newPass.length() == 0 || newMqttHost.length() == 0) {
      m_webPage += "<form method='post' action='setting'><label><p>Server SSID:     </label><input name='ssid' length=" + String(wifi_MaxSsidLength) + "></p>";
      m_webPage += "<p>Server password: <input name='pass' length=" + String(wifi_MaxPasswordLength) + "input type='password'> </p>";
      m_webPage += "<br><p>ESP camera IP address: <input name='oct1' length=3 input type='number' style='width: 50px'>.";
      m_webPage += "<input name='oct2' length=3 input type='number' style='width: 50px'>.<input name='oct3' length=3 input type='number' style='width: 50px'>.";
      m_webPage += "<input name='oct4' length=3 input type='number' style='width: 50px'></p>";
      m_webPage += "<p>MQTT Server username: " + String(mqtt_Username.c_str()) + "</p>";
      m_webPage += "<p>MQTT Server host address: <input name='mqtt_host' length=" + String(mqtt_MaxMqttHostLength) + "> Port: <input name='mqtt_port' length=5 style='width: 70px'></p>";
      m_webPage += "<p>MQTT Server password: <input name='mqtt_pass' length=" + String(mqtt_MaxMqttPasswordLength) + " input type='password'></p>";
      m_webPage += "<p><button>Save</button></p></form>";
      m_webPage += "<br><p>Empty server credentials, retry.</p>";
      m_webServer.sendHeader("Access-Control-Allow-Credentials", "true");
      m_webServer.send(200, "text/html", m_webPage);
      return;
    }

    if (connectToDestination(newSsid, newPass)) {
      updateCredentials(newSsid, newPass, newMqttHost, newMqttPass, newMqttPort);
      WiFi.config(newIp, WiFi.gatewayIP(), WiFi.subnetMask());
      m_webPage += "<p>Credentials saved. Rebooting...</p></html>";
      m_webServer.sendHeader("Access-Control-Allow-Credentials", "true");
      m_webServer.send(200, "text/html", m_webPage);
      delay(100);
      ESP.restart();
    } else {
      m_webPage += "<form method='post' action='setting'><label><p>Server SSID:     </label><input name='ssid' length=" + String(wifi_MaxSsidLength) + "></p>";
      m_webPage += "<p>Server password: <input name='pass' length=" + String(wifi_MaxPasswordLength) + "input type='password'> </p>";
      m_webPage += "<br><p>ESP camera IP address: <input name='oct1' length=3 input type='number' style='width: 50px'>.";
      m_webPage += "<input name='oct2' length=3 input type='number' style='width: 50px'>.<input name='oct3' length=3 input type='number' style='width: 50px'>.";
      m_webPage += "<input name='oct4' length=3 input type='number' style='width: 50px'></p>";
      m_webPage += "<p>MQTT Server username: " + String(mqtt_Username.c_str()) + "</p>";
      m_webPage += "<p>MQTT Server host address: <input name='mqtt_host' length=" + String(mqtt_MaxMqttHostLength) + "> Port: <input name='mqtt_port' length=5 style='width: 70px'></p>";
      m_webPage += "<p>MQTT Server password: <input name='mqtt_pass' length=" + String(mqtt_MaxMqttPasswordLength) + " input type='password'></p>";
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

void WifiManager::updateCredentials(const string newSsid, const string newPass, const string newMqttHost, const string newMqttPass, const string newMqttPort)
{
  // read old MQTT credentials
  constexpr uint8_t portLength = 5;
  string mqttHost, mqttPass, mqttPort;
  mqttHost.reserve(mqtt_MaxMqttHostLength);
  mqttPass.reserve(mqtt_MaxMqttPasswordLength);
  mqttPort.reserve(portLength);
  for(int i = 0; i < mqtt_MaxMqttHostLength; i++) {
    mqttHost[i] = EEPROM.read(i + wifi_MaxSsidLength + wifi_MaxPasswordLength);
    if (mqttHost[i] == 0) {
      break;
    }
  }

  for(int i = 0; i < mqtt_MaxMqttPasswordLength; i++) {
    mqttPass[i] = EEPROM.read(i + wifi_MaxSsidLength + wifi_MaxPasswordLength + mqtt_MaxMqttHostLength);
    if (mqttPass[i] == 0) {
      break;
    }
  }

  for(int i = 0; i < portLength; i++) {
    mqttPort[i] = EEPROM.read(i + wifi_MaxSsidLength + wifi_MaxPasswordLength + mqtt_MaxMqttHostLength + mqtt_MaxMqttPasswordLength);
    if (mqttPort[i] == 0) {
      break;
    }
  }
  

  // clear old SSID characters
  for (int i = 0; i < wifi_MaxSsidLength; i++) {
    writeToEeprom(i, 0);
  }

  // clear old password characters
  for (int i = 0; i < wifi_MaxPasswordLength; i++) {
    writeToEeprom(wifi_MaxSsidLength + i, 0);
  }

  // clear old MQTT host characters
  for (int i = 0; i < mqtt_MaxMqttHostLength; i++) {
    writeToEeprom(wifi_MaxSsidLength + wifi_MaxPasswordLength + i, 0);
  }

  // clear old MQTT password characters, if needed
  for (int i = 0; i < mqtt_MaxMqttPasswordLength; i++) {
    writeToEeprom(wifi_MaxSsidLength + wifi_MaxPasswordLength + 
                  mqtt_MaxMqttHostLength + i, 0);
  }

  // clear old MQTT port characters
  for (int i = 0; i < mqtt_MaxPortLength; i++) {
    writeToEeprom(wifi_MaxSsidLength + wifi_MaxPasswordLength + 
                  mqtt_MaxMqttHostLength + i, 0);
  }

  // update new credentials
  for (int i = 0; i < newSsid.length(); i++) {
    writeToEeprom(i, newSsid[i]);
  }

  for (int i = 0; i < newPass.length(); i++) {
    writeToEeprom(i + wifi_MaxSsidLength, newPass[i]);
  }

  for (int i = 0; i < newMqttHost.length(); i++) {
    writeToEeprom(i + wifi_MaxSsidLength + wifi_MaxPasswordLength, newMqttHost[i]);
  }

  for (int i = 0; i < newMqttPass.length(); i++) {
    writeToEeprom(i + wifi_MaxSsidLength + wifi_MaxPasswordLength + mqtt_MaxMqttHostLength, newMqttPass[i]);
  }

  for (int i = 0; i < newMqttPort.length(); i++) {
    writeToEeprom(i + wifi_MaxSsidLength + wifi_MaxPasswordLength + mqtt_MaxMqttHostLength + mqtt_MaxMqttPasswordLength, newMqttPort[i]);
  }

  EEPROM.commit();
}
