#pragma once

template <typename T>
class Service
{
  inline static T mInstance;

public:
  static T& Get() { return mInstance; }
};
