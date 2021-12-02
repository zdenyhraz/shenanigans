#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "FITS.h"
#include "IPC/IPC.h"
#include "Optimization/Optimization_.h"
#include "Log/logger.h"

static const std::vector<std::string> WAVELENGTHS_STR = {"HMI", "94_AIA", "131_AIA", "171_AIA", "171_SECCHIA", "171_SECCHIB", "193_AIA", "195_SECCHIA", "195_SECCHIB", "211_AIA", "284_SECCHIA",
    "284_SECCHIB", "304_AIA", "304_SECCHIA", "304_SECCHIB", "335_AIA"};
static const std::vector<double> STDDEVS(WAVELENGTHS_STR.size(), 0);

double absoluteSubpixelRegistrationError(IPCsettings& set, const Mat& src, double noisestddev, double maxShift, double accuracy);

double IPCparOptFun(std::vector<double>& args, const IPCsettings& settingsMaster, const Mat& source, double noisestddev, double maxShift, double accuracy);

void optimizeIPCParameters(const IPCsettings& settingsMaster, std::string pathInput, std::string pathOutput, double maxShift, double accuracy, unsigned runs);

void optimizeIPCParametersForAllWavelengths(const IPCsettings& settingsMaster, double maxShift, double accuracy, unsigned runs);
