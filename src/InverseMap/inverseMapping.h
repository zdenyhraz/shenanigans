#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"

std::vector<f64> inverseMappingZH(const std::vector<std::vector<f64>>& trialInputs, const std::vector<std::vector<f64>>& trialOutputs, const std::vector<f64>& desiredOutput, i32 degree = 1);

std::vector<f64> inverseMappingTestTransfer(std::vector<f64> arg);