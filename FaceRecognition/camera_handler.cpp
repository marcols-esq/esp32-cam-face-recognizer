#include "esp_camera.h"
#include "fd_forward.h"
#include "fr_forward.h"

#include "camera_handler.h"
#include "matrix3d.h"
#include "config.h"

CameraHandler::CameraHandler()
{
  m_config.ledc_channel = camera_LedcChannel;
  m_config.ledc_timer = camera_LedcTimer;
  m_config.pin_d0 = pin_Y2;
  m_config.pin_d1 = pin_Y3;
  m_config.pin_d2 = pin_Y4;
  m_config.pin_d3 = pin_Y5;
  m_config.pin_d4 = pin_Y6;
  m_config.pin_d5 = pin_Y7;
  m_config.pin_d6 = pin_Y8;
  m_config.pin_d7 = pin_Y9;
  m_config.pin_xclk = pin_XCLK;
  m_config.pin_pclk = pin_PCLK;
  m_config.pin_vsync = pin_VSYNC;
  m_config.pin_href = pin_HREF;
  m_config.pin_sscb_sda = pin_SIOD;
  m_config.pin_sscb_scl = pin_SIOC;
  m_config.pin_pwdn = pin_PWDN;
  m_config.pin_reset = pin_RST;
  m_config.xclk_freq_hz = camera_XclkFrequency;
  m_config.pixel_format = camera_PixelFormat;
  m_config.jpeg_quality = camera_Quality;
  m_config.fb_count = camera_FbCount;
  m_config.frame_size = camera_FrameSize;
  esp_camera_init(&m_config);

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
}

void CameraHandler::allocateFrameBuffer() { m_frameBuffer = esp_camera_fb_get(); }

camera_fb_t* CameraHandler::getFrameBuffer() { return m_frameBuffer; }

void CameraHandler::releaseFrameBuffer() { esp_camera_fb_return(m_frameBuffer); }

const bool CameraHandler::convertFrameToRgb888(dl_matrix3du_t* outputBuffer)
{
  return fmt2rgb888(m_frameBuffer->buf, m_frameBuffer->len, m_frameBuffer->format, outputBuffer->item);
}

void CameraHandler::enableFlashLight() { digitalWrite(4, HIGH); }

void CameraHandler::disableFlashLight() { digitalWrite(4, LOW); }
