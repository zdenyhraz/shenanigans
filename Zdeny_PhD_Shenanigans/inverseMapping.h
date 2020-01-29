#pragma once
#include "functionsBaseSTL.h"
#include "functionsBaseCV.h"

std::vector<double> inverseMappingZH(const std::vector<std::vector<double>>& trialInputs, const std::vector<std::vector<double>>& trialOutputs, const std::vector<double>& desiredOutput, int degree = 1);

std::vector<double> inverseMappingTestTransfer(std::vector<double> arg);