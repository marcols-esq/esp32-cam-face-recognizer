#include "mqtt_handler.h"
#include <EEPROM.h>

using namespace std;
using namespace std::chrono;

// initialization of static variables
esp_mqtt_client_handle_t MqttHandler::m_mqttClient = NULL;
std::array<MqttMessage, mqtt_MaxNumberOfTopics> MqttHandler::m_subscribedTopics {};

// must be static, hence static variables in class
static esp_err_t handleEvent(esp_mqtt_event_handle_t event)
{
  switch (event->event_id)
  {
  case MQTT_EVENT_DATA:
    Serial.printf("MqttHandler: Received data in topic %.*s: %.*s\n", event->topic_len, event->topic, event->data_len, event->data);
    MqttHandler::updateMessage(event->topic, event->topic_len, event->data, event->data_len);
    break;

  case MQTT_EVENT_DISCONNECTED:
    MqttHandler::attemptReconnection();
    break;

  default:
    break;
  }
  return ESP_OK;
}

// used for trimming strings returned by MQTT handler
void trimString(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
      return !std::isspace(ch);
  }).base(), s.end());
  s.shrink_to_fit();
}

MqttHandler::MqttHandler()
{
  m_mqttHost.reserve(mqtt_MaxMqttHostLength);
  m_mqttPass.reserve(mqtt_MaxMqttPasswordLength);
  m_mqttPort.reserve(mqtt_MaxPortLength);

  uint32_t readingOffset = wifi_MaxSsidLength + wifi_MaxPasswordLength;
  
  for(int i = 0; i < mqtt_MaxMqttHostLength; i++) {
    m_mqttHost += EEPROM.read(i + readingOffset);
    if (m_mqttHost[i] == 0) {
      break;
    }
  }
  readingOffset += mqtt_MaxMqttHostLength;

  for(int i = 0; i < mqtt_MaxMqttUsernameLength; i++) {
    m_mqttUsername += EEPROM.read(i + readingOffset);
    if (m_mqttUsername[i] == 0) {
      break;
    }
  }
  readingOffset += mqtt_MaxMqttUsernameLength;

  for(int i = 0; i < mqtt_MaxMqttPasswordLength; i++) {
    m_mqttPass += EEPROM.read(i + readingOffset);
    if (m_mqttPass[i] == 0) {
      break;
    }
  }
  readingOffset += mqtt_MaxMqttPasswordLength;

  for(int i = 0; i < mqtt_MaxPortLength; i++) {
    m_mqttPort += EEPROM.read(i + readingOffset);
    if (m_mqttPort[i] == 0) {
      break;
    }
  }

  m_mqttConfig = {0};
  m_mqttConfig.host = m_mqttHost.c_str();
  m_mqttConfig.port = atoi(m_mqttPort.c_str());
  m_mqttConfig.event_handle = handleEvent;
  m_mqttConfig.username = m_mqttUsername.c_str();
  m_mqttConfig.password = m_mqttPass.c_str();
  m_mqttConfig.transport = MQTT_TRANSPORT_OVER_TCP;

  m_mqttClient = esp_mqtt_client_init(&m_mqttConfig);

  for (int i = 0; i < m_subscribedTopics.size(); i++) {
    m_subscribedTopics[i].topic.clear();
    m_subscribedTopics[i].msg.clear();
  }
}

bool MqttHandler::start()
{
  Serial.printf("MqttHandler: Connecting to MQTT server: %s:%d with username: %s\n", m_mqttHost.c_str(), atoi(m_mqttPort.c_str()), m_mqttUsername.c_str());
  return esp_mqtt_client_start(m_mqttClient) == ESP_OK;
}

bool MqttHandler::subscribeToTopic(string topic, uint8_t qosLevel)
{
  // if the topic is not in m_subscribedTopics
  if (!getTopicIndex(topic)) {
    if (esp_mqtt_client_subscribe(m_mqttClient, topic.c_str(), qosLevel) != -1) {
      addTopic(topic);
      Serial.printf("MqttHandler: Subscribed to topic: %s\n", topic.c_str());
      return true;
    }
    Serial.printf("MqttHandler: Failed subscribing to topic: %s\n", topic.c_str());
  }
  
  return false;
}

bool MqttHandler::unsubscribeFromTopic(string topic)
{
  uint8_t topicIndex;
  if (getTopicIndex(topic, &topicIndex)) {
    if (esp_mqtt_client_unsubscribe(m_mqttClient, topic.c_str()) != -1) {
      removeTopic(topicIndex);
      return true;
    }
  }
  
  return false;
}

bool MqttHandler::publishMessage(string topic, string msg, uint8_t qosLevel)
{
  return esp_mqtt_client_publish(m_mqttClient, topic.c_str(), msg.c_str(), msg.length(), qosLevel, 0) != 0;
}

string MqttHandler::getLatestMessage(std::string topic)
{
  uint8_t topicIndex;
  if (!getTopicIndex(topic, &topicIndex)) {
    Serial.printf("MqttHandler: Tried to read message from non-subscribed topic: %s\n", topic.c_str());
    return string("");
  }
  
  return m_subscribedTopics[topicIndex].msg;
}

bool MqttHandler::getTopicIndex(string topic, uint8_t* resultDestination)
{
  for (int i = 0; i < m_subscribedTopics.size(); i++) {
    if (m_subscribedTopics[i].topic == topic) {
      if (resultDestination != nullptr) {
        *resultDestination = i;
      }
      return true;
    }
  }

  return false;
}

bool MqttHandler::addTopic(string topic)
{
  for (int i = 0; i < m_subscribedTopics.size(); i++) {
    if (m_subscribedTopics[i].topic.empty()) {
      m_subscribedTopics[i].topic = topic;
      return true;
    }
  }

  Serial.println("MqttHandler: Max number of subscribed topics reached");
  return false;
}

void MqttHandler::removeTopic(uint8_t topicIndex)
{
  if (topicIndex < m_subscribedTopics.size() - 1) {
    m_subscribedTopics[topicIndex].topic.clear();
    m_subscribedTopics[topicIndex].msg.clear();
  }
}

void MqttHandler::updateMessage(const char* topic, uint32_t topicLength, const char* msg, uint32_t msgLength)
{
  string topicStr(topic, topicLength);
  string messageStr(msg, msgLength);
  trimString(topicStr);
  trimString(messageStr);
  
  uint8_t topicIndex;
  if (getTopicIndex(topicStr, &topicIndex)) {
    m_subscribedTopics[topicIndex].msg = messageStr;
    return;
  }
  
  Serial.printf("MqttHandler: Tried to update message in not subscribed topic: %s\n", topicStr.c_str());
}

void MqttHandler::attemptReconnection()
{
  Serial.println("MqttHandler: Attempting to reconnect to MQTT server");
  esp_mqtt_client_reconnect(m_mqttClient);
}

void MqttHandler::stopClient()
{
  // In used version of mqtt_client.h, usage of this function causes a deadlock. Fun.
  // esp_mqtt_client_stop(m_mqttClient);
}
