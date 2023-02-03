#pragma once
#include "Window.hpp"

class RandomWindow : public Window
{
  void EvolutionOptimization(bool meta);
  void UnevenIlluminationCLAHE();
  void UnevenIlluminationHomomorphic();

public:
  void Render() override;
};
