#pragma once
#include "stdafx.h"
#include "IPC.h"

void alignPics( const Mat &input1, const Mat &input2, Mat &output, IPCsettings set );

Mat AlignStereovision( const Mat &img1In, const Mat &img2In );

void alignPicsDebug( const Mat &img1In, const Mat &img2In, IPCsettings &IPC_settings );

void registrationDuelDebug( IPCsettings &IPC_settings1, IPCsettings &IPC_settings2 );

std::tuple<Mat, Mat> calculateFlowMap( const Mat &img1In, const Mat &img2In, IPCsettings &IPC_settings, double qualityRatio );