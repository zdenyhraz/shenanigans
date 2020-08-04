#pragma once
#include "Core/functionsBaseSTL.h"
#include "Core/functionsBaseCV.h"
#include "Log/logger.h"

using namespace std;

struct OptimizationAlgorithm//the main parent optimizer class
{
	int N = 1;//the problem dimension
	vector<double> lowerBounds = zerovect( N, -1. ); //lower search space bounds
	vector<double> upperBounds = zerovect( N, +1. ); //upper search space bounds
	double optimalFitness = 0;//satisfactory function value
	int funEvals = 0;//current # of function evaluations
	int maxFunEvals = 1e10;//maximum # of function evaluations
	int maxGen = 1000;//maximum # of algorithm iterations
	bool success = false;//success in reaching satisfactory function value
	bool logPoints = false;//switch for logging of explored points
	vector<vector<vector<double>>> visitedPoints;//the 3D vector of visited points [run][iter][dim]
	string terminationReason = "optimization not run yet";//the reason for algorithm termination
	OptimizationAlgorithm( int N ) : N( N ), lowerBounds( zerovect( N, -1. ) ), upperBounds( zerovect( N, 1. ) ) {}; //construct some default bounds

	virtual std::vector<double> optimize( std::function<double( std::vector<double> )> f ) = 0;
};

struct Evolution : OptimizationAlgorithm
{
	enum class MutationStrategy : char { RAND1, BEST1, RAND2, BEST2 };
	enum class CrossoverStrategy : char { BIN, EXP };
	enum class StoppingCriterion : char { ALLIMP, AVGIMP};
	int NP = 4;
	double F = 0.65;
	double CR = 0.95;
	double iNPm = 8;
	MutationStrategy mutStrat = MutationStrategy::RAND1;
	CrossoverStrategy crossStrat = CrossoverStrategy::BIN;
	StoppingCriterion stopCrit = StoppingCriterion::AVGIMP;
	int distincEntityMaxTrials = 0;
	int historySize = 10;
	double historyImprovTresholdPercent = 1;
	Evolution( int N ) : OptimizationAlgorithm( N ), NP( iNPm * N ) {};

	std::vector<double> optimize( std::function<double( std::vector<double> )> f ) override;
};

struct PatternSearch : OptimizationAlgorithm
{
	double minStep = 1e-5;
	int multistartMaxCnt = 1;
	int multistartCnt = 0;
	int maxExploitCnt = 0;
	double stepReducer = 0.5;

	PatternSearch( int N ) : OptimizationAlgorithm( N ) {};

	vector<double> optimize( std::function<double( vector<double> )> f ) override;
};

struct Simplex : OptimizationAlgorithm
{
	Simplex( int N ) : OptimizationAlgorithm( N ) {};

	vector<double> optimize( std::function<double( vector<double> )> f );
};

struct MCS : OptimizationAlgorithm
{
	MCS( int N ) : OptimizationAlgorithm( N ) {};

	vector<double> optimize( std::function<double( vector<double> )> f );
};

namespace optimizationTestFunctions {
inline double Sphere( vector<double> arg )
{
	double returnVal = 0;
	for ( int i = 0; i < arg.size(); i++ )
	{
		returnVal += pow( ( arg[i] ), 2 );
	}
	return returnVal;
}

inline double Ackley( vector<double> arg )
{
	double x = arg[0];
	double y = arg[1];
	return -20 * exp( -0.2 * sqrt( 0.5 * ( x * x + y * y ) ) ) - exp( 0.5 * ( cos( 2 * Constants::Pi * x ) + cos( 2 * Constants::Pi * y ) ) ) + exp( 1. ) + 20;
}

inline double Himmelblau( vector<double> arg )
{
	double x = arg[0];
	double y = arg[1];
	return pow( x * x + y - 11, 2 ) + pow( x + y * y - 7, 2 );
}

inline double Rosenbrock( vector<double> arg )
{
	double x = arg[0];
	double y = arg[1];
	return 100 * pow( y - x * x, 2 ) + pow( 1 - x, 2 );
}

inline double Beale( vector<double> arg )
{
	double x = arg[0];
	double y = arg[1];
	return pow( 1.5 - x + x * y, 2 ) + pow( 2.25 - x + x * y * y, 2 ) + pow( 2.625 - x + x * y * y * y, 2 );
}
}

#ifdef OPTIMIZE_WITH_CV
using namespace cv;

Mat drawFunc2D( std::function<double( vector<double> )> f, double xmin, double xmax, double ymin, double ymax, int stepsX, int stepsY );

inline void drawPoint2D( const Mat &funcLandscape, cv::Point point, double stretchFactorX, double stretchFactorY, cv::Scalar CrosshairColor = Scalar( 255 * 0.7, 0, 255 * 0.7 ) )
{
	//Scalar(255 * 0.7, 0, 255 * 0.7) - magenta
	int linePxLength = max( funcLandscape.cols / 120, 1 );
	int thickness = max( funcLandscape.cols / 200, 1 );
	point.x *= stretchFactorX;
	point.y *= stretchFactorY;
	cv::Point NW( point.x - linePxLength, point.y - linePxLength );
	cv::Point NE( point.x + linePxLength, point.y - linePxLength );
	cv::Point SW( point.x - linePxLength, point.y + linePxLength );
	cv::Point SE( point.x + linePxLength, point.y + linePxLength );
	line( funcLandscape, NW, SE, CrosshairColor, thickness );
	line( funcLandscape, NE, SW, CrosshairColor, thickness );
}

inline void drawArrow2D( const Mat &funcLandscape, cv::Point point1, cv::Point point2, double stretchFactorX, double stretchFactorY, cv::Scalar CrosshairColor = Scalar( 0, 0, 255 ) )
{
	point1.x *= stretchFactorX;
	point1.y *= stretchFactorY;
	point2.x *= stretchFactorX;
	point2.y *= stretchFactorY;
	int thickness = max( funcLandscape.cols / 400, 1 );
	arrowedLine( funcLandscape, point1, point2, CrosshairColor, thickness );
}

Mat drawPath2D( Mat funcLandscape, vector<vector<vector<double>>> points, double xmin, double xmax, double ymin, double ymax, int stepsX, int stepsY, double stretchFactorX, double stretchFactorY, bool drawArrows = 1 );

vector<double> drawFuncLandscapeAndOptimize2D( std::function<double( vector<double> )> f, vector<double> lowerBounds, vector<double> upperBounds, vector<int> steps, Evolution Evo, PatternSearch Pat, bool logLandscapeOpt = 1, bool optPat = 0, bool optEvo = 0, double quantileB = 0, double quantileT = 1, Mat *landscape = nullptr );

void optimizeWithLandscapeDebug();

#endif