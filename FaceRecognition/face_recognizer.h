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
#include "fd_forward.h"
#include "fr_forward.h"
#include "image_util.h"

#include "camera_handler.h"
#include "matrix3d.h"

class FaceRecognizer
{
public:
  FaceRecognizer();
  const bool detect();
  const int8_t recognize();
  const int8_t enrollFace();
  void enableFlashlight();
  

private:
  CameraHandler m_camera;
  mtmn_config_t m_faceDetectionConfig {0};
  box_array_t* m_boundingBox = NULL;
  Matrix3D m_imageMatrix;
  Matrix3D m_faceMatrix;
  face_id_list m_enrolledFaceList {NULL};

  void clearBoundingBox();
  const int8_t enrollFace(dl_matrix3du_t* faceMatrix);
};
