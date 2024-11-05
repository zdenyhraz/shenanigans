#pragma once
#include "Window.hpp"

class RandomWindow : public Window
{
  static void EvolutionOptimization(bool meta);
  static void UnevenIlluminationCLAHE();
  static void UnevenIlluminationHomomorphic();

public:
  void Render() override;
};
