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
#include "dl_lib_matrix3d.h"
#include "dl_lib_matrix3dq.h"


class Matrix3D
{
public:
  Matrix3D() = default;
  
  ~Matrix3D()
  {
    if(m_matrix != NULL)
    {
      dl_matrix3du_free(m_matrix);
    }
  }

  void createMatrix(const uint8_t width, const uint8_t height, const uint8_t channel)
  {
    m_matrix = dl_matrix3du_alloc(1, width, height, channel);
    if(!m_matrix) {
      Serial.println("Matrix3D: Could not allocate memory for the matrix.\n");
    }
  }

  void destroyMatrix()
  {
    dl_matrix3du_free(m_matrix);
    m_matrix = NULL;
  }

  dl_matrix3du_t* getMatrix() { return m_matrix; }

private:
  dl_matrix3du_t* m_matrix = NULL;
};
