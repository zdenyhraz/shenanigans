#include "stdafx.h"
#include "inverseMapping.h"

std::vector<double> inverseMappingZH( const std::vector<std::vector<double>> &trialInputs, const std::vector<std::vector<double>> &trialOutputs, const std::vector<double> &desiredOutput, int degree )
{
	//assuming input dimension is the same as output dimension for existence and uniqueness
	int N = trialInputs[0].size();
	int trials = trialInputs.size();

	//degenerate safety checks
	if ( trials < ( degree * N + 1 ) ) //underdetermined system
		return zerovect( N, 0. );

	if ( trialInputs.size() != trialOutputs.size() ) //system makes no sense
		return zerovect( N, 0. );

	if ( trialInputs[0].size() != trialOutputs[0].size() ) //zero or infinite solutions
		return zerovect( N, 0. );

	//fill input trial matrix
	Mat trialInputsM = Mat::zeros( trials, degree * N + 1, CV_32F );
	for ( int r = 0; r < trials; r++ )
	{
		for ( int deg = 0; deg < degree; deg++ )
		{
			for ( int c = deg * N; c < ( deg + 1 ) * N; c++ )
			{
				trialInputsM.at<float>( r, c ) = pow( trialInputs[r][c % N], deg + 1 );
			}
		}
		trialInputsM.at<float>( r, trialInputsM.cols - 1 ) = 1; //constant term, last
	}

	//fill output trial matrix
	Mat trialOutputsM = Mat::zeros( trials, degree * N + 1, CV_32F );
	for ( int r = 0; r < trials; r++ )
	{
		for ( int deg = 0; deg < degree; deg++ )
		{
			for ( int c = deg * N; c < ( deg + 1 ) * N; c++ )
			{
				if ( deg == 0 )
					trialOutputsM.at<float>( r, c ) = trialOutputs[r][c % N];
				else
					trialOutputsM.at<float>( r, c ) = 1;
			}
		}
		trialOutputsM.at<float>( r, trialOutputsM.cols - 1 ) = 1; //constant term, last
	}

	Mat transferMatrix = ( trialInputsM.t() * trialInputsM ).inv( DECOMP_LU ) * trialInputsM.t() * trialOutputsM; //calculate transfer matrix
	Mat desiredOutputM = Mat::zeros( 1, degree * N + 1, CV_32F );
	for ( int deg = 0; deg < degree; deg++ )
	{
		for ( int c = deg * N; c < ( deg + 1 ) * N; c++ )
		{
			if ( deg == 0 )
				desiredOutputM.at<float>( 0, c ) = desiredOutput[c % N];
			else
				desiredOutputM.at<float>( 0, c ) = 1;
		}
	}
	desiredOutputM.at<float>( 0, desiredOutputM.cols - 1 ) = 1; //constant term, last

	Mat desiredInputM = desiredOutputM * transferMatrix.inv( DECOMP_LU ); //calculate desired input matrix
	std::vector<double> desiredInput( N, 0 );
	for ( int c = 0; c < N; c++ )
		desiredInput[c] = desiredInputM.at<float>( 0, c );
	return desiredInput;
}

std::vector<double> inverseMappingTestTransfer( std::vector<double> arg )
{
	auto result = arg;
	auto N = arg.size();

	if ( N == 4 )
	{
		if ( 1 ) //linear
		{
			result[0] = 1 * arg[0] + 4 * arg[1] + -2 * arg[2] + 3 * arg[3] - 1;
			result[1] = -3 * arg[0] + 1 * arg[1] + 2 * arg[2] + -7 * arg[3] + 3;
			result[2] = 6 * arg[0] + 5 * arg[1] + -9 * arg[2] + 2 * arg[3] + -7;
			result[3] = -13 * arg[0] + -0.5 * arg[1] + 6 * arg[2] + 1 * arg[3] + 10;
		}
		if ( 0 ) //quadratic
		{
			result[0] = 1 * arg[0] - 3.5 * arg[1] + -2.5 * arg[2] + 3 * arg[3] + 1 * pow( arg[0], 2 ) + -3 * pow( arg[1], 2 ) + 2 * pow( arg[2], 2 ) + 5 * pow( arg[3], 2 ) - 1;
			result[1] = -3 * arg[0] + 1 * arg[1] + 2 * arg[2] + -7 * arg[3] + 5 * pow( arg[0], 2 ) + 8 * pow( arg[1], 2 ) - 10 * pow( arg[2], 2 ) + -2 * pow( arg[3], 2 ) - 3;
			result[2] = 2 * arg[0] + 5 * arg[1] + -9.5 * arg[2] + 4 * arg[3] + 3 * pow( arg[0], 2 ) + 8 * pow( arg[1], 2 ) + 6 * pow( arg[2], 2 ) + 1 * pow( arg[3], 2 ) + 7;
			result[3] = -13 * arg[0] + -0.5 * arg[1] + 6 * arg[2] + 1.5 * arg[3] + 11 * pow( arg[0], 2 ) + 3 * pow( arg[1], 2 ) - 8 * pow( arg[2], 2 ) - 2 * pow( arg[3], 2 ) - 2;
		}
	}
	else
	{
		if ( 1 ) //linear
			result[0] = 5 * arg[0] + 7;
		if ( 0 ) //quadratic
			result[0] = 5 * arg[0] + -3 * pow( arg[0], 2 ) + 7;
	}

	return result;
}