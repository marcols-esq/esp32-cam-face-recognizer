//  MIT License
//
//  Copyright (c) 2022 Marcin Olszowy
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "Arduino.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include <string>
#include <WebServer.h>

class WifiManager
{
public:
  WifiManager();
  bool connectToDestination(const std::string ssid = std::string(""), const std::string password = std::string(""));
  void runSetupServer();
  void disconnect();

private:
  void setDestinationConnectionParameters(const std::string ssid, const std::string password);
  void enableSetupServer();
  bool disableSetupServer();
  void setupWebPage();
  void updateCredentials(const std::string newSsid, 
                         const std::string newPass, 
                         const std::string newMqttHost, 
                         const std::string newMqttUsername,
                         const std::string newMqttPass, 
                         const std::string newMqttPort, 
                         const std::string newMqttControlTopic, 
                         const std::string newMqttResultTopic);
  
  const std::string SetupSsid = "ESP Face recognizer";
  const std::string SetupPassword = "admin";
  
  std::string m_destinationSsid;
  std::string m_destinationPassword;
  WiFiClient m_accessPointClient;
  WebServer m_webServer {80};
  String m_webPage;
  bool m_isSetupServerEnabled;
};