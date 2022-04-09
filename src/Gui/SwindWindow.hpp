#pragma once

#pragma once
#include "Features/FeatureMatch.hpp"

class SwindWindow
{
public:
  static void Initialize();
  static void Render();

private:
  inline static FeatureMatchData mParameters;
};
