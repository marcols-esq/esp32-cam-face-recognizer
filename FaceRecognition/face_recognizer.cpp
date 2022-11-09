#include "config.h"
#include "face_recognizer.h"

FaceRecognizer::FaceRecognizer()
{
  m_faceDetectionConfig = mtmn_init_config();
  face_id_init(&m_enrolledFaceList, recognizer_MaxNumberOfFaces, recognizer_EnrollmentSamples);
  read_face_id_from_flash(&m_enrolledFaceList);
}

const bool FaceRecognizer::detect()
{
  m_camera.allocateFrameBuffer();
  m_imageMatrix.createMatrix(m_camera.getFrameBuffer()->width, m_camera.getFrameBuffer()->height, 3);
  m_camera.convertFrameToRgb888(m_imageMatrix.getMatrix());
  m_camera.releaseFrameBuffer();
  m_boundingBox = face_detect(m_imageMatrix.getMatrix(), &m_faceDetectionConfig);
  if(m_boundingBox) {
    return true;
  } else {
    m_imageMatrix.destroyMatrix();
    return false;
  }
}

const int8_t FaceRecognizer::recognize()
{
  int8_t id = 0;
  if(m_boundingBox != NULL) {
    m_faceMatrix.createMatrix(recognizer_FaceWidth, recognizer_FaceHeight, 3);
    if(align_face(m_boundingBox, m_imageMatrix.getMatrix(), m_faceMatrix.getMatrix()) == ESP_OK) {
      id = recognize_face(&m_enrolledFaceList, m_faceMatrix.getMatrix());
      clearBoundingBox();
    }
    m_faceMatrix.destroyMatrix();
  } else {
    id = 127;
  }
  m_imageMatrix.destroyMatrix();
  return id;
}

const int8_t FaceRecognizer::enrollFace()
{
    m_faceMatrix.createMatrix(recognizer_FaceWidth, recognizer_FaceHeight, 3);
    if(align_face(m_boundingBox, m_imageMatrix.getMatrix(), m_faceMatrix.getMatrix()) == ESP_OK) {
      int samplesLeft = enroll_face_id_to_flash(&m_enrolledFaceList, m_faceMatrix.getMatrix());
      if (samplesLeft >= 0) {
        Serial.printf("FaceRecognizer: Enrolling id: %d, Samples left: %d\n", m_enrolledFaceList.tail, samplesLeft);
      } else if (samplesLeft == -2) {
        Serial.println("FaceRecognizer: Could not find flash partition");
      }
      clearBoundingBox();
      m_faceMatrix.destroyMatrix();
      m_imageMatrix.destroyMatrix();
      if (samplesLeft == 0) {
        Serial.printf("FaceRecognizer: Enrolled id: %d\n", m_enrolledFaceList.tail);
        return m_enrolledFaceList.tail;
      } 
    }
  return 127;
}

void FaceRecognizer::enableFlashlight()
{
  m_camera.enableFlashLight();
}

void FaceRecognizer::disableFlashlight()
{
  m_camera.disableFlashLight();
}

void FaceRecognizer::clearBoundingBox()
{
  free(m_boundingBox->score);
  free(m_boundingBox->box);
  free(m_boundingBox->landmark);
  free(m_boundingBox);
  m_boundingBox = NULL;
}

const uint8_t FaceRecognizer::deleteFace()
{
  const uint8_t res = delete_face_id_in_flash(&m_enrolledFaceList);
  m_enrolledFaceList.tail--;
  return res;
}
