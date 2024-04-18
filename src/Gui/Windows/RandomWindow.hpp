#pragma once
#include "Window.hpp"

class RandomWindow : public Window
{
  void EvolutionOptimization(bool meta) const;
  void UnevenIlluminationCLAHE() const;
  void UnevenIlluminationHomomorphic() const;
  void MicroserviceTestManual() const;
  void MicroserviceTestNodes() const;

public:
  void Render() override;
};
