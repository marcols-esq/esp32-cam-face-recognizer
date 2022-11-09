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
#include <array>
#include "config.h"
#include "mqtt_client.h"
#include <string>

struct MqttMessage
{
  std::string topic;
  std::string msg;
};

class MqttHandler
{
public:
  MqttHandler();
  bool start();
  bool subscribeToTopic(std::string topic, uint8_t qosLevel = 0);
  bool unsubscribeFromTopic(std::string topic);
  bool publishMessage(std::string topic, std::string msg, uint8_t qosLevel = 0);
  std::string getLatestMessage(std::string topic);
  static void updateMessage(const char* topic, uint32_t topicLength, const char* msg, uint32_t msgLength);
  static void attemptReconnection();
  void stopClient();

private:
  static bool getTopicIndex(std::string topic, uint8_t* resultDestination = nullptr);
  bool addTopic(std::string topic);
  void removeTopic(uint8_t topicIndex);
  
  esp_mqtt_client_config_t m_mqttConfig;
  static esp_mqtt_client_handle_t m_mqttClient;
  static std::array<MqttMessage, mqtt_MaxNumberOfTopics> m_subscribedTopics;
  std::string m_mqttHost;
  std::string m_mqttPass;
  std::string m_mqttPort;
};
