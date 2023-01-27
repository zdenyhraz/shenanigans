#pragma once
#include "Window.hpp"
#include "Features/FeatureMatch.hpp"

class SwindWindow : public Window
{
  FeatureMatchData mParameters;

public:
  void Render() override;
};
