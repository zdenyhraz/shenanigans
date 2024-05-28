#pragma once
#include "Window.hpp"

class RandomWindow : public Window
{
  void EvolutionOptimization(bool meta) const;
  void UnevenIlluminationCLAHE() const;
  void UnevenIlluminationHomomorphic() const;

public:
  void Render() override;
};
