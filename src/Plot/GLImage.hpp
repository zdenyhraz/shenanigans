#pragma once
#include "Gui/Gui.hpp"

class GLImage
{
public:
  mutable GLuint texid = 0;

  GLImage() {}

  ~GLImage()
  {
    if (texid != 0)
      glDeleteTextures(1, &texid);
  }

  static GLuint GetGLDataType(const cv::Mat& z)
  {
    switch (z.type())
    {
    case CV_8U:
      return GL_UNSIGNED_BYTE;
    case CV_8UC3:
      return GL_UNSIGNED_BYTE;
    case CV_16U:
      return GL_UNSIGNED_SHORT;
    case CV_16UC3:
      return GL_UNSIGNED_SHORT;
    case CV_32F:
      return GL_FLOAT;
    case CV_32FC3:
      return GL_FLOAT;
    }
    throw std::runtime_error("Invalid mat type");
  }

  void Load(const cv::Mat& z) const
  {
    PROFILE_FUNCTION;
    LOG_FUNCTION;
    CheckGLError("GLImage::Load(const cv::Mat& z)");
    glGenTextures(1, &texid);
    CheckGLError("glGenTextures");
    glBindTexture(GL_TEXTURE_2D, texid);
    CheckGLError("glBindTexture");

    if (true)
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      CheckGLError("glTexParameteri");
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      CheckGLError("glTexParameteri");

      // #if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
      //       glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
      //       CheckGLError("glPixelStorei");
      // #endif
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, z.cols, z.rows, 0, GL_BGR, GetGLDataType(z), z.data);
    CheckGLError("glTexImage2D");
  }
};
