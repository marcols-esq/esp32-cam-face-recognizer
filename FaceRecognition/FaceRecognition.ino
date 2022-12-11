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


#include "camera_index.h"
#include <chrono>
#include <EEPROM.h>
#include "esp_camera.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "img_converters.h"

#include "face_recognizer.h"
#include "wifi_manager.h"
#include "mqtt_handler.h"

using namespace std;
using namespace std::chrono;

string GetRecognitionControlTopic();
string GetRecognitionResultTopic();

void setup() {  
  constexpr uint32_t BaudRate = 115200;
  Serial.begin(BaudRate);
  
  FaceRecognizer recognizer;
  
  WifiManager wifiManager;
  if (!wifiManager.connectToDestination()) {
    wifiManager.runSetupServer(); // has internal while(1)
  }
  
  bool detected;
  int id = 0;
  long heartbeat = 0;
  char recognitionResultMsgBuffer[32] = "";
  milliseconds time = milliseconds(0);
  milliseconds heartbeatCycle = seconds(10);
  MqttHandler mqttHandler;  // should be created after creating WifiManager
  int enrollmentCycles = 0;
  
  while(mqttHandler.start() != true);
  delay(1000);  // wait for MQTT server to ackgnowledge connection
  uint8_t retries = mqtt_ConnectionRetriesBeforeTimeout;
  while (!mqttHandler.subscribeToTopic(GetRecognitionControlTopic())) {
    Serial.println("Could not connect to MQTT server, retrying in 5 seconds");
    if (!retries) {
      mqttHandler.stopClient();
      wifiManager.disconnect();
      wifiManager.runSetupServer();
    }
    
    retries--;
    delay(5000);
  }
  
  mqttHandler.subscribeToTopic(GetRecognitionResultTopic());
  
  while (1) {
    if (mqttHandler.getLatestMessage(GetRecognitionControlTopic()) == "run") {
      recognizer.enableFlashlight();  
      if (mqttHandler.getLatestMessage(GetRecognitionResultTopic()) == "enroll") {
        enrollmentCycles = recognizer_EnrollmentSamples;
      } else if (mqttHandler.getLatestMessage(GetRecognitionResultTopic()) == "delete") {
        recognizer.deleteFace();
      } 
           
      detected = recognizer.detect(); 
      if (detected) {
        Serial.println("Face detected.");
        // if the device is in enrolling process
        if (enrollmentCycles > 0) {
          id = recognizer.enrollFace();
          enrollmentCycles--;
          sprintf(recognitionResultMsgBuffer, "Enrolling new face, samples left: %d", enrollmentCycles);
          mqttHandler.publishMessage(GetRecognitionResultTopic(), string(recognitionResultMsgBuffer));
          continue;
        }
              
        id = recognizer.recognize();
        if (id >= 0 && id < 127) {
          Serial.printf("Face ID: %d\n\n", id);
          sprintf(recognitionResultMsgBuffer, "Detected face, face id: %d", id);
        } else if (id == 127) {
          Serial.println("Error occured when recognizing face\n");
          sprintf(recognitionResultMsgBuffer, "Error occured when recognizing face");
        } else {
          Serial.println("Face not recognized\n");
          sprintf(recognitionResultMsgBuffer, "Detected face, not recognized");
        }
        
        mqttHandler.publishMessage(GetRecognitionResultTopic(), string(recognitionResultMsgBuffer));
      }
      
    } else if (mqttHandler.getLatestMessage(GetRecognitionControlTopic()) == "stop") {
      recognizer.disableFlashlight();
      if (milliseconds(millis()) - time > heartbeatCycle) {
        Serial.printf("Face recognition stopped. Heartbeat: %d\n", heartbeat++);
        time = milliseconds(millis());
      }
    } else {
      if (milliseconds(millis()) - time > heartbeatCycle) {
        recognizer.disableFlashlight();
        Serial.printf("Invalid/no message received from MQTT server. Heartbeat: %d\n", heartbeat++);
        time = milliseconds(millis());
      }
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}

string GetRecognitionControlTopic() {
  static string recognitionControlTopic;

  if (recognitionControlTopic.empty()) {
    uint32_t readingOffset = wifi_MaxSsidLength + wifi_MaxPasswordLength +
                             mqtt_MaxMqttHostLength + mqtt_MaxMqttUsernameLength +
                             mqtt_MaxMqttPasswordLength + mqtt_MaxPortLength;
  
    for(int i = 0; i < mqtt_MaxMqttRecognitionControlTopicLength; i++) {
      recognitionControlTopic += EEPROM.read(i + readingOffset);
      if (recognitionControlTopic[i] == 0) {
        recognitionControlTopic.pop_back();
        break;
      }
    }
  }
  
  return recognitionControlTopic;
}

string GetRecognitionResultTopic() {
  static string recognitionResultTopic;

  if (recognitionResultTopic.empty()) {
    uint32_t readingOffset = wifi_MaxSsidLength + wifi_MaxPasswordLength +
                             mqtt_MaxMqttHostLength + mqtt_MaxMqttUsernameLength +
                             mqtt_MaxMqttPasswordLength + mqtt_MaxPortLength +
                             mqtt_MaxMqttRecognitionControlTopicLength;
                             
    for(int i = 0; i < mqtt_MaxMqttRecognitionResultTopicLength; i++) {
      recognitionResultTopic += EEPROM.read(i + readingOffset);
      if (recognitionResultTopic[i] == 0) {
        recognitionResultTopic.pop_back();
        break;
      }
    }
  }

  return recognitionResultTopic;
}