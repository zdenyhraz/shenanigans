#pragma once

#ifdef GLAPI
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
    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, z.cols, z.rows, 0, GL_BGR, GetGLDataType(z), z.data);
  }
};
#endif
