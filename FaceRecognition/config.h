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
#include "esp_camera.h"
#include "fd_forward.h"
#include "fr_forward.h"

static const uint8_t recognizer_MaxNumberOfFaces  = 7;
static const uint8_t recognizer_EnrollmentSamples = 5;
static const uint8_t recognizer_EnrollTimeToLeave = 20;
static const uint8_t recognizer_FaceWidth         = FACE_WIDTH;
static const uint8_t recognizer_FaceHeight        = FACE_HEIGHT;


static const uint8_t pin_PWDN   = 32;
static const int8_t  pin_RST    = -1;
static const uint8_t pin_XCLK   = 0;
static const uint8_t pin_SIOD   = 26;
static const uint8_t pin_SIOC   = 27;
static const uint8_t pin_Y9     = 35;
static const uint8_t pin_Y8     = 34;
static const uint8_t pin_Y7     = 39;
static const uint8_t pin_Y6     = 36;
static const uint8_t pin_Y5     = 21;
static const uint8_t pin_Y4     = 19;
static const uint8_t pin_Y3     = 18;
static const uint8_t pin_Y2     = 5;
static const uint8_t pin_VSYNC  = 25;
static const uint8_t pin_HREF   = 23;
static const uint8_t pin_PCLK   = 22;


static const ledc_channel_t camera_LedcChannel  = LEDC_CHANNEL_0;
static const ledc_timer_t camera_LedcTimer      = LEDC_TIMER_0;
static const uint64_t camera_XclkFrequency      = 20000000;
static const pixformat_t camera_PixelFormat     = PIXFORMAT_JPEG;
static const uint8_t camera_Quality             = 10;
static const uint8_t camera_FbCount             = 2;
static const framesize_t camera_FrameSize       = FRAMESIZE_HQVGA;
