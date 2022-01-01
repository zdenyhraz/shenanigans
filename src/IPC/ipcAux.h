#pragma once

#include "IPC.h"

void alignPics(const cv::Mat& input1, const cv::Mat& input2, cv::Mat& output, IPCsettings set);

cv::Mat AlignStereovision(const cv::Mat& img1In, const cv::Mat& img2In);

void alignPicsDebug(const cv::Mat& img1In, const cv::Mat& img2In, IPCsettings& IPC_settings);

void registrationDuelDebug(IPCsettings& IPC_settings1, IPCsettings& IPC_settings2);

std::tuple<cv::Mat, cv::Mat> calculateFlowMap(const cv::Mat& img1In, const cv::Mat& img2In, IPCsettings& IPC_settings, f64 qualityRatio);