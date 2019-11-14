#pragma once
#include "functionsBaseSTL.h"
#include "functionsBaseCV.h"

std::vector<double> inverseMappingZH(std::vector<std::vector<double>>& trialInputs, std::vector<std::vector<double>>& trialOutputs, std::vector<double>& desiredOutput, int degree = 1, std::ofstream* listing = nullptr);

std::vector<double> inverseMappingTestTransfer(std::vector<double> arg);