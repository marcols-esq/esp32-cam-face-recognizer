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
#include "esp_camera.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "img_converters.h"

#include "face_recognizer.h"
#include "wifi_manager.h"

enum class Mode { setup, runtime };

bool enroll = false;
Mode mode;

void setup() {
constexpr uint32_t BaudRate = 115200;
Serial.begin(BaudRate);

FaceRecognizer recognizer;
pinMode(12, INPUT_PULLUP);

WifiManager wifiManager;
if (wifiManager.connectToDestination()) {
  mode = Mode::runtime;
} else {
  mode = Mode::setup;
}

bool detected;
int id = 0;
  while (1) {
    switch (mode){
    case Mode::runtime:
      detected = recognizer.detect(); 
      if (detected) {
        Serial.println("Face detected.");
        switch (enroll) {
        case true:
          id = recognizer.enrollFace();
          break;
        case false:
          id = recognizer.recognize();
    
          if (id >= 0 && id < 127) {
            Serial.printf("Face ID: %d\n\n", id);
            enroll = false;
          } else if (id == 127) {
            Serial.println("Error occured when recognizing face.\n");
          } else {
            Serial.println("Face not recognized.\n");
          }
    
          break;
        }
      }
    
      if (digitalRead(12) == LOW){
        enroll = true;
      }
      break;
    case Mode::setup:
      wifiManager.runSetupServer();
      break;
    }       
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
