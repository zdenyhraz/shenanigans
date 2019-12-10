#pragma once
#include "functionsBaseSTL.h"
#include "logger.h"

#ifdef OPT_WITH_CV
#include "functionsBaseCV.h"
#endif

using namespace std;

enum class MutationStrategy : char { RAND1, BEST1, RAND2, BEST2 };
enum class CrossoverStrategy : char { BIN, EXP };
enum class StoppingCriterion : char { ALLIMPROVBOOL, ALLIMPROVPERC, AVGIMPROVPERC };
enum class LpNorm : char { L1, L2 };

struct OptimizationAlgorithm//the main parent optimizer class
{
	int N = 1;//the problem dimension
	vector<double> lowerBounds = zerovect(N, -1);//lower search space bounds
	vector<double> upperBounds = zerovect(N, 1);//upper search space bounds
	double optimalFitness = std::numeric_limits<double>::lowest();//satisfactory function value
	int funEvals = 0;//current # of function evaluations
	int maxFunEvals = 1e10;//maximum # of function evaluations
	int maxGen = 1000;//maximum # of algorithm iterations
	bool success = false;//success in reaching satisfactory function value
	bool logPointsMain = false;//switch for sparse logging of explored points
	bool logPointsAll = false;//switch for dense logging of explored points
	vector<vector<vector<double>>> visitedPointsMain;//the 3D sparse vector of visited points [run][iter][dim]
	vector<vector<vector<double>>> visitedPointsAll;//the 3D dense vector of visited points [run][iter][dim]
	string terminationReason = "optimization not run yet";//the reason for algorithm termination
	OptimizationAlgorithm(int N) : N(N), lowerBounds(zerovect(N, -1)), upperBounds(zerovect(N, 1)) {};
};

inline double averageVectorDistance(std::vector<double>& vec1, std::vector<double>& vec2, std::vector<double>& boundsRange)
{
	double result = 0;
	for (int i = 0; i < vec1.size(); i++)
	{
		result += abs(vec1[i] - vec2[i]) / boundsRange[i];//normalize -> 0 to 1
	}
	result /= vec1.size();//coordinate average
	return result;
}

inline bool isDistinct(int inpindex, std::vector<int>& indices, int currindex)
{
	bool isdist = true;
	for (auto& idx : indices)
	{
		if (inpindex == idx || inpindex == currindex)
			isdist = false;
	}
	return isdist;
}

struct Evolution : OptimizationAlgorithm
{
	int NP = 4;
	double F = 0.65;//large is more exploration
	double CR = 0.95;//close to 1 for dependent variables close to 0 for independent variables (separable / non-separable objective function)
	double iNPm = 8;
	MutationStrategy mutStrat = MutationStrategy::RAND1;
	CrossoverStrategy crossStrat = CrossoverStrategy::BIN;
	StoppingCriterion stopCrit = StoppingCriterion::AVGIMPROVPERC;
	int distincEntityMaxTrials = 0;//spread the initial population abit
	int historySize = 10;
	double historyImprovTresholdPercent = 1;
	std::function<void(std::vector<double>)> OnGenerationSUBEVENT;

	Evolution(int N) : OptimizationAlgorithm(N), NP(iNPm*N) {};

	std::vector<double> optimize(std::function<double(std::vector<double>)> f, Logger* logger = nullptr)
	{
		if (logger) logger->LogMessage("Optimization started (evolution)", EVENT);
		
		vector<vector<double>> visitedPointsMainThisRun;
		vector<vector<double>> visitedPointsAllThisRun;
		double averageImprovement = 0;
		funEvals = 0;
		success = false;
		vector<double> boundsRange = upperBounds - lowerBounds;

		//establish population matrix, fitness vector and other intrinsics
		vector<vector<double>> population(NP, zerovect(N));
		vector<queue<double>> histories(NP);
		bool historyConstant = false;
		vector<double> fitness = zerovect(NP);
		vector<double> bestEntity = zerovect(N);
		double bestFitness = std::numeric_limits<double>::max();
		double fitness_prev = std::numeric_limits<double>::max();
		double fitness_curr = std::numeric_limits<double>::max();
		success = false;

		//initialize random starting population matrix within bounds
		if (logger) logger->LogMessage("Initializing population within bounds ... ", SUBEVENT);
		double initialMinAvgDist = 0.5;
		for (int indexEntity = 0; indexEntity < NP; indexEntity++)
		{
			int distinctEntityTrials = 0;
			bool distinctEntity = false;//entities may not be too close together
			while (!distinctEntity)//loop until they are distinct enough
			{
				distinctEntity = true;//assume entity is distinct
				distinctEntityTrials++;
				for (int indexParam = 0; indexParam < N; indexParam++)//generate initial entity
				{
					population[indexEntity][indexParam] = randInRange(lowerBounds[indexParam], upperBounds[indexParam]);
				}
				if (distincEntityMaxTrials == 0) break;
				for (int indexEntity2 = 0; indexEntity2 < indexEntity; indexEntity2++)//check distance to all other entities
				{
					double avgDist = averageVectorDistance(population[indexEntity], population[indexEntity2], boundsRange);//calculate how distinct the entity is to another entity
					if (logger) logger->LogMessage("entity" + to_string(indexEntity) + " trial " + to_string(distinctEntityTrials) + " avgDist to entity" + to_string(indexEntity2) + ": " + to_string(avgDist) + ", minimum dist: " + to_string(initialMinAvgDist), INFO);
					if (avgDist < initialMinAvgDist)//check if entity is distinct
					{
						distinctEntity = false;
						if (logger) logger->LogMessage("entity" + to_string(indexEntity) + " is not distinct to entity " + to_string(indexEntity2) + ", bruh moment", INFO);
						break;//needs to be distinct from all entities
					}
				}
				if (distinctEntityTrials >= distincEntityMaxTrials)
				{
					initialMinAvgDist *= 0.8;
					distinctEntityTrials = 0;
				}
			}
		}

		if (logger) logger->LogMessage("Initial population created", SUBEVENT);
		//calculate initial fitness vector
		#pragma omp parallel for
		for (int indexEntity = 0; indexEntity < NP; indexEntity++)
		{
			fitness[indexEntity] = f(population[indexEntity]);
			if (logPointsAll)
			{
				#pragma omp critical
				visitedPointsAllThisRun.push_back(population[indexEntity]);
			}
		}
		funEvals += NP;

		//run main evolution cycle
		for (int generation = 1; generation < 1e8; generation++)
		{
			#pragma omp parallel for
			for (int indexEntity = 0; indexEntity < NP; indexEntity++)
			{
				//create new potential entity
				vector<double> newEntity(N, 0.);
				newEntity = population[indexEntity];
				double newFitness = 0.;

				//select distinct parents different from the current entity
				int numberOfParents;
				//calculate the number of parents
				switch (mutStrat)
				{
				case MutationStrategy::RAND1: numberOfParents = 3; break;
				case MutationStrategy::BEST1: numberOfParents = 2; break;
				case MutationStrategy::RAND2: numberOfParents = 5; break;
				case MutationStrategy::BEST2: numberOfParents = 4; break;
				}
				vector<int> parentIndices(numberOfParents, 0);
				for (auto& idx : parentIndices)
				{
					int idxTst;
					do { idxTst = rand() % NP; } while (!isDistinct(idxTst, parentIndices, indexEntity));
					idx = idxTst;
				}

				//decide which parameters undergo crossover
				vector<bool> paramIsCrossed(N, false);
				switch (crossStrat)
				{
				case CrossoverStrategy::BIN:
				{
					int definite = rand() % N;//at least one param undergoes crossover
					for (int indexParam = 0; indexParam < N; indexParam++)
					{
						double random = randInRange();
						if (random < CR || indexParam == definite) paramIsCrossed[indexParam] = true;
					}
					break;
				}
				case CrossoverStrategy::EXP:
				{
					int L = 0;
					do { L++; } while ((randInRange() < CR) && (L < N));//at least one param undergoes crossover
					int indexParam = rand() % N;
					for (int i = 0; i < L; i++)
					{
						paramIsCrossed[indexParam] = true;
						indexParam++;
						indexParam %= N;
					}
					break;
				}
				}

				//perform the crossover
				for (int indexParam = 0; indexParam < N; indexParam++)
				{
					if (paramIsCrossed[indexParam])
					{
						switch (mutStrat)
						{
						case MutationStrategy::RAND1: newEntity[indexParam] = population[parentIndices[0]][indexParam] + F * (population[parentIndices[1]][indexParam] - population[parentIndices[2]][indexParam]); break;
						case MutationStrategy::BEST1: newEntity[indexParam] = bestEntity[indexParam] + F * (population[parentIndices[0]][indexParam] - population[parentIndices[1]][indexParam]); break;
						case MutationStrategy::RAND2: newEntity[indexParam] = population[parentIndices[0]][indexParam] + F * (population[parentIndices[1]][indexParam] - population[parentIndices[2]][indexParam]) + F * (population[parentIndices[3]][indexParam] - population[parentIndices[4]][indexParam]); break;
						case MutationStrategy::BEST2: newEntity[indexParam] = bestEntity[indexParam] + F * (population[parentIndices[0]][indexParam] - population[parentIndices[1]][indexParam]) + F * (population[parentIndices[2]][indexParam] - population[parentIndices[3]][indexParam]); break;
						}
					}
					//check for boundaries, effectively clamp
					newEntity[indexParam] = clampSmooth(newEntity[indexParam], population[indexEntity][indexParam], lowerBounds[indexParam], upperBounds[indexParam]);
				}

				//evaluate fitness of new entity
				newFitness = f(newEntity);

				if (logPointsAll)
				{
					#pragma omp critical 
					visitedPointsAllThisRun.push_back(newEntity);
				}

				//select the more fit entity
				if (newFitness <= fitness[indexEntity])
				{
					population[indexEntity] = newEntity;
					fitness[indexEntity] = newFitness;
				}
			}//entity cycle end
			funEvals += NP;

			//determine the best entity
			for (int indexEntity = 0; indexEntity < NP; indexEntity++)
			{
				if (fitness[indexEntity] <= bestFitness)
				{
					bestEntity = population[indexEntity];
					bestFitness = fitness[indexEntity];
					fitness_prev = fitness_curr;
					fitness_curr = bestFitness;
					if (logPointsMain) visitedPointsMainThisRun.push_back(bestEntity);
					if (logger && ((fitness_prev - fitness_curr) / fitness_prev * 100 > 2)) 
					{ 
						logger->LogMessage("Gen " + to_string(generation) + " best entity: " + to_string(bestFitness), INFO);
						logger->LogMessage("CBI = " + to_string((fitness_prev - fitness_curr) / fitness_prev * 100) + "%, AHI = " + to_string(averageImprovement * 100) + "%", DEBUG);
					}
				}
			}

			//fill history ques for all entities - termination criterion
			historyConstant = true;//assume history is constant
			averageImprovement = 0;
			for (int indexEntity = 0; indexEntity < NP; indexEntity++)
			{
				if (histories[indexEntity].size() == historySize)
				{
					histories[indexEntity].pop();//remove first element - keep que size constant
					histories[indexEntity].push(fitness[indexEntity]);//insert at the end
					if (stopCrit == StoppingCriterion::ALLIMPROVBOOL) { if (histories[indexEntity].front() != histories[indexEntity].back()) historyConstant = false; } //fitness is constant for all entities
					if (stopCrit == StoppingCriterion::ALLIMPROVPERC) { if (abs(histories[indexEntity].front() - histories[indexEntity].back()) / abs(histories[indexEntity].front()) > historyImprovTresholdPercent / 100) historyConstant = false; }//fitness improved less than x% for all entities
				}
				else
				{
					histories[indexEntity].push(fitness[indexEntity]);//insert at the end
					historyConstant = false;//too early to stop, go on
				}
				if (histories[indexEntity].size() > 2) averageImprovement += abs(histories[indexEntity].front()) == 0 ? 0 : abs(histories[indexEntity].front() - histories[indexEntity].back()) / abs(histories[indexEntity].front());
			}
			averageImprovement /= NP;
			if (stopCrit == StoppingCriterion::AVGIMPROVPERC) { if (100 * averageImprovement > historyImprovTresholdPercent) historyConstant = false; } //average fitness improved less than x%

			if (OnGenerationSUBEVENT)
				OnGenerationSUBEVENT({static_cast<double>(generation),bestFitness, averageImprovement});

			//termination criterions
			if (bestFitness < optimalFitness)//fitness goal reached
			{
				if (logger) logger->LogMessage("OptimalFitness value reached, terminating - generation " + to_string(generation) + ".", SUBEVENT);
				terminationReason = "optimalFitness value reached, final fitness: " + to_string(bestFitness);
				success = true;
				break;
			}
			if (generation == maxGen)//maximum generation reached
			{
				if (logger) logger->LogMessage("MaxGen value reached, terminating - generation " + to_string(generation) + ".", SUBEVENT);
				terminationReason = "maxGen value reached, final fitness: " + to_string(bestFitness);
				success = false;
				break;
			}
			if (funEvals >= maxFunEvals)//maximum function evaluations exhausted
			{
				if (logger) logger->LogMessage("MaxFunEvals value reached, terminating - generation " + to_string(generation) + ".", SUBEVENT);
				terminationReason = "maxFunEvals value reached, final fitness: " + to_string(bestFitness);
				success = false;
				break;
			}
			if (historyConstant)//no entity improved last (historySize) generations
			{
				if (logger) logger->LogMessage("historyConstant value reached, terminating - generation " + to_string(generation) + ".", SUBEVENT);
				terminationReason = "historyConstant value reached, final fitness: " + to_string(bestFitness);
				success = false;
				break;
			}
		}//generation cycle end
		if (logPointsMain) visitedPointsMain.push_back(visitedPointsMainThisRun);
		if (logPointsAll) visitedPointsAll.push_back(visitedPointsAllThisRun);
		return bestEntity;
	}//optimize function end
};

struct PatternSearch : OptimizationAlgorithm
{
	double minStep = 1e-5;
	int multistartMaxCnt = 1;
	int multistartCnt = 0;
	int maxExploitCnt = 0;
	double stepReducer = 0.5;

	PatternSearch(int N) : OptimizationAlgorithm(N) {};

	vector<double> optimize(std::function<double(vector<double>)> f, Logger* logger = nullptr)
	{
		//if (logger) logger->LogMessage    ">> Optimization started (pattern search)"  ;

		vector<double> boundsRange = upperBounds - lowerBounds;
		double initialStep = vectorMax(boundsRange) / 4;
		funEvals = 0;
		multistartCnt = 0;
		//multistart algorithm - initialize global results     
		vector<double> topPoint = zerovect(N);
		double topPointFitness = std::numeric_limits<double>::max();
		//generate all starting points
		vector<vector<double>> mainPointsInitial(multistartMaxCnt, zerovect(N));
		for (int run = 0; run < multistartMaxCnt; run++)
		{
			for (int indexParam = 0; indexParam < N; indexParam++)
			{
				mainPointsInitial[run][indexParam] = randInRange(lowerBounds[indexParam], upperBounds[indexParam]);//idk dude
			}
		}

		//multistart pattern search
		volatile bool flag = false;
		#pragma omp parallel for shared(flag)
		for (int run = 0; run < multistartMaxCnt; run++)
		{
			if (flag) continue;

			vector<vector<double>> visitedPointsAllThisRun;
			vector<vector<double>> visitedPointsMainThisRun;
			int funEvalsThisRun = 0;
			//initialize vectors
			double step = initialStep;
			vector<double> mainPoint = mainPointsInitial[run];
			double mainPointFitness = f(mainPoint);
			if (logPointsMain) visitedPointsMainThisRun.push_back(mainPoint);
			if (logPointsAll) visitedPointsAllThisRun.push_back(mainPoint);
			vector<vector<vector<double>>> pattern;//N-2-N (N pairs of N-dimensional points)
			vector<vector<double>> patternFitness(N, zerovect(2, std::numeric_limits<double>::max()));//N-2 (N pairs of fitness)
			pattern.resize(N);
			for (int dim = 0; dim < N; dim++)
			{
				pattern[dim].resize(2);
				for (int pm = 0; pm < 2; pm++)
				{
					pattern[dim][pm].resize(N);
				}
			}

			//main search cycle
			for (int generation = 1; generation < 1e8; generation++)
			{
				bool smallerStep = true;
				//asign values to pattern vertices - exploration
				for (int dim = 0; dim < N; dim++)
				{
					for (int pm = 0; pm < 2; pm++)
					{
						pattern[dim][pm] = mainPoint;
						pattern[dim][pm][dim] += pm == 0 ? step : -step;
						pattern[dim][pm][dim] = clampSmooth(pattern[dim][pm][dim], mainPoint[dim], lowerBounds[dim], upperBounds[dim]);

						//evaluate vertices
						patternFitness[dim][pm] = f(pattern[dim][pm]);
						funEvalsThisRun++;

						if (logPointsAll) visitedPointsAllThisRun.push_back(pattern[dim][pm]);

						//select best pattern vertex and replace
						if (patternFitness[dim][pm] < mainPointFitness)
						{
							mainPoint = pattern[dim][pm];
							mainPointFitness = patternFitness[dim][pm];
							if (logPointsMain) visitedPointsMainThisRun.push_back(pattern[dim][pm]);
							smallerStep = false;
							//if (logger) logger->LogMessage  "> run "  run  " current best entity fitness: "  patternFitness[dim][pm]  ;
							if (maxExploitCnt > 0)
							{
								double testPointFitness = mainPointFitness;
								for (int exploitCnt = 0; exploitCnt < maxExploitCnt; exploitCnt++)
								{
									vector<double> testPoint = mainPoint;
									testPoint[dim] += pm == 0 ? step : -step;
									testPoint[dim] = clampSmooth(testPoint[dim], mainPoint[dim], lowerBounds[dim], upperBounds[dim]);
									testPointFitness = f(testPoint);
									funEvalsThisRun++;
									if (logPointsAll) visitedPointsAllThisRun.push_back(testPoint);
									if (testPointFitness < mainPointFitness)
									{
										mainPoint = testPoint;
										mainPointFitness = testPointFitness;
										if (logPointsMain) visitedPointsMainThisRun.push_back(testPoint);
										//if (logger) logger->LogMessage  "> run "  run  " - exploitation "  exploitCnt  " just improved the fitness: "  testPointFitness  ;
									}
								}
							}
						}
					}
				}

				//no improvement - mainPoint is the best point - lower the step size
				if (smallerStep)
					step *= stepReducer;

				//termination criterions
				if (step < minStep)
				{
					#pragma omp critical
					{
						//if (logger) logger->LogMessage  "> minStep value reached, terminating - generation "  generation  "."  ;
						success = false;
						terminationReason = "minStep value reached, final fitness: " + to_string(mainPointFitness);
					}
					break;
				}
				if (mainPointFitness < optimalFitness)
				{
					#pragma omp critical
					{
						//if (logger) logger->LogMessage  "> optimalFitness value reached, terminating - generation "  generation  "."  ;
						success = true;
						terminationReason = "optimalFitness value reached, final fitness: " + to_string(mainPointFitness);
					}
					break;
				}
				if (generation == maxGen)
				{
					#pragma omp critical
					{
						//if (logger) logger->LogMessage  "> maxGen value reached, terminating - generation "  generation  "."  ;
						success = false;
						terminationReason = "maxGen value reached, final fitness: " + to_string(mainPointFitness);
					}
					break;
				}
				if ((funEvals >= maxFunEvals) || (funEvalsThisRun >= maxFunEvals))
				{
					#pragma omp critical
					{
						//if (logger) logger->LogMessage  "MaxFunEvals value reached, terminating - generation "  generation  "."  ;
						terminationReason = "maxFunEvals value reached, final fitness: " + to_string(mainPointFitness);
						success = false;
						flag = true;//dont do other runs, out of funEvals
					}
					break;
				}
			}//generations end

			//multistart result update
			#pragma omp critical
			{
				multistartCnt++;
				funEvals += funEvalsThisRun;
				if (logPointsMain) visitedPointsMain.push_back(visitedPointsMainThisRun);
				if (logPointsAll) visitedPointsAll.push_back(visitedPointsAllThisRun);
				//if (logger) logger->LogMessage  "> run "  run  ": ";
				if (mainPointFitness < topPointFitness)
				{
					topPoint = mainPoint;
					topPointFitness = mainPointFitness;
					//if (logger) showEntity(topPoint, topPointFitness, "current multistart best", true, true);
					if (topPointFitness < optimalFitness) flag = true;//dont do other runs, fitness goal reached
				}
				else
				{
					//if (logger) logger->LogMessage  "- run has ended with no improvement, fitness: "  mainPointFitness  ;
				}
			}

		}//multistart end
		return topPoint;
	}//opt end

};

struct Simplex : OptimizationAlgorithm
{
	Simplex(int N) : OptimizationAlgorithm(N) {};

	vector<double> optimize(std::function<double(vector<double>)> f)
	{
		return zerovect(N);
	}
};

struct MCS : OptimizationAlgorithm
{
	MCS(int N) : OptimizationAlgorithm(N) {};

	vector<double> optimize(std::function<double(vector<double>)> f)
	{
		return zerovect(N);
	}
};

namespace optimizationTestFunctions
{
	inline double Sphere(vector<double> arg)
	{
		double returnVal = 0;
		for (int i = 0; i < arg.size(); i++)
		{
			returnVal += pow((arg[i]), 2);
		}
		return returnVal;
	}

	inline double Ackley(vector<double> arg)
	{
		double x = arg[0];
		double y = arg[1];
		return -20 * exp(-0.2*sqrt(0.5*(x*x + y * y))) - exp(0.5*(cos(2 * PI*x) + cos(2 * PI*y))) + exp(1.) + 20;
	}

	inline double Himmelblau(vector<double> arg)
	{
		double x = arg[0];
		double y = arg[1];
		return pow(x*x + y - 11, 2) + pow(x + y * y - 7, 2);
	}

	inline double Rosenbrock(vector<double> arg)
	{
		double x = arg[0];
		double y = arg[1];
		return 100 * pow(y - x * x, 2) + pow(1 - x, 2);
	}

	inline double Beale(vector<double> arg)
	{
		double x = arg[0];
		double y = arg[1];
		return pow(1.5 - x + x * y, 2) + pow(2.25 - x + x * y*y, 2) + pow(2.625 - x + x * y*y*y, 2);
	}
}

#ifdef OPT_WITH_CV
using namespace cv;

Mat drawFunc2D(std::function<double(vector<double>)> f, double xmin, double xmax, double ymin, double ymax, int stepsX, int stepsY);

inline void drawPoint2D(Mat& funcLandscape, cv::Point point, double stretchFactorX, double stretchFactorY, cv::Scalar CrosshairColor = Scalar(255 * 0.7, 0, 255 * 0.7))
{
	//Scalar(255 * 0.7, 0, 255 * 0.7) - magenta
	int linePxLength = max(funcLandscape.cols / 120, 1);
	int thickness = max(funcLandscape.cols / 200, 1);
	point.x *= stretchFactorX;
	point.y *= stretchFactorY;
	cv::Point NW(point.x - linePxLength, point.y - linePxLength);
	cv::Point NE(point.x + linePxLength, point.y - linePxLength);
	cv::Point SW(point.x - linePxLength, point.y + linePxLength);
	cv::Point SE(point.x + linePxLength, point.y + linePxLength);
	line(funcLandscape, NW, SE, CrosshairColor, thickness);
	line(funcLandscape, NE, SW, CrosshairColor, thickness);
}

inline void drawArrow2D(Mat& funcLandscape, cv::Point point1, cv::Point point2, double stretchFactorX, double stretchFactorY, cv::Scalar CrosshairColor = Scalar(0, 0, 255))
{
	point1.x *= stretchFactorX;
	point1.y *= stretchFactorY;
	point2.x *= stretchFactorX;
	point2.y *= stretchFactorY;
	int thickness = max(funcLandscape.cols / 400, 1);
	arrowedLine(funcLandscape, point1, point2, CrosshairColor, thickness);
}

Mat drawPath2D(Mat funcLandscape, vector<vector<vector<double>>> points, double xmin, double xmax, double ymin, double ymax, int stepsX, int stepsY, double stretchFactorX, double stretchFactorY, bool drawArrows = 1);

vector<double> drawFuncLandscapeAndOptimize2D(std::function<double(vector<double>)> f, vector<double> lowerBounds, vector<double> upperBounds, vector<int> steps, Evolution Evo, PatternSearch Pat, bool logLandscapeOpt = 1, bool optPat = 0, bool optEvo = 0, double quantileB = 0, double quantileT = 1, Mat* landscape = nullptr);

void optimizeWithLandscapeDebug();

#endif