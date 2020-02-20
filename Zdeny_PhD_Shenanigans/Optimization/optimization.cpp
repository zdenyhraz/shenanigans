//various optimization functions and features
//updated frequently @ https://github.com/zdenyhraz
//PhD work of Zdenek Hrazdira
//made during 2018-2019

#include "stdafx.h"
#include "optimization.h"

std::vector<double> Evolution::optimize(std::function<double(std::vector<double>)> f, Logger* logger, IPlot1D* plt)
{
	if (logger) logger->Log("Optimization started (evolution)", EVENT);

	vector<vector<double>> visitedPointsMainThisRun;
	vector<vector<double>> visitedPointsThisRun;
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
	if (logger) logger->Log("Initializing population within bounds ... ", SUBEVENT);
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
				if (logger) logger->Log("entity" + to_string(indexEntity) + " trial " + to_string(distinctEntityTrials) + " avgDist to entity" + to_string(indexEntity2) + ": " + to_string(avgDist) + ", minimum dist: " + to_string(initialMinAvgDist), DEBUG);
				if (avgDist < initialMinAvgDist)//check if entity is distinct
				{
					distinctEntity = false;
					if (logger) logger->Log("entity" + to_string(indexEntity) + " is not distinct to entity " + to_string(indexEntity2) + ", bruh moment", DEBUG);
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

	if (logger) logger->Log("Initial population created\n", SUBEVENT);
	//calculate initial fitness vector
	#pragma omp parallel for
	for (int indexEntity = 0; indexEntity < NP; indexEntity++)
	{
		fitness[indexEntity] = f(population[indexEntity]);
		if (logPoints)
		{
			#pragma omp critical
			visitedPointsThisRun.push_back(population[indexEntity]);
		}
	}
	funEvals += NP;

	if (plt) plt->setAxisNames("generation", "fitness", "log (fitness)");
	if (plt) plt->clear(true);

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

			if (logPoints)
			{
				#pragma omp critical 
				visitedPointsThisRun.push_back(newEntity);
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
				if (logger && ((fitness_prev - fitness_curr) / fitness_prev * 100 > 2))
				{
					logger->Log("Gen " + to_string(generation) + " best entity: " + to_string(bestFitness), DEBUG);
					logger->Log("CBI = " + to_string((fitness_prev - fitness_curr) / fitness_prev * 100) + "%, AHI = " + to_string(averageImprovement * 100) + "%", DEBUG);
					if ((fitness_prev - fitness_curr) / fitness_prev * 100 > 25) logger->Log("Big improvement!", SUBEVENT);
					logger->Log("", DEBUG);
				}
			}
		}

		if (plt) plt->plot(generation, bestFitness, log(bestFitness));
		
		//fill history ques for all entities - termination criterion
		historyConstant = true;//assume history is constant
		averageImprovement = 0;
		for (int indexEntity = 0; indexEntity < NP; indexEntity++)
		{
			if (histories[indexEntity].size() == historySize)
			{
				histories[indexEntity].pop();//remove first element - keep que size constant
				histories[indexEntity].push(fitness[indexEntity]);//insert at the end
				if (stopCrit == StoppingCriterion::ALLIMP) { if (abs(histories[indexEntity].front() - histories[indexEntity].back()) / abs(histories[indexEntity].front()) > historyImprovTresholdPercent / 100) historyConstant = false; }//fitness improved less than x% for all entities
			}
			else
			{
				histories[indexEntity].push(fitness[indexEntity]);//insert at the end
				historyConstant = false;//too early to stop, go on
			}
			if (histories[indexEntity].size() > 2) averageImprovement += abs(histories[indexEntity].front()) == 0 ? 0 : abs(histories[indexEntity].front() - histories[indexEntity].back()) / abs(histories[indexEntity].front());
		}
		averageImprovement /= NP;
		if (stopCrit == StoppingCriterion::AVGIMP) { if (100 * averageImprovement > historyImprovTresholdPercent) historyConstant = false; }//average fitness improved less than x%

		//termination criterions
		if (bestFitness < optimalFitness)//fitness goal reached
		{
			if (logger) logger->Log("OptimalFitness value reached, terminating - generation " + to_string(generation) + ".\n", SUBEVENT);
			terminationReason = "optimalFitness value reached, final fitness: " + to_string(bestFitness);
			success = true;
			break;
		}
		if (generation == maxGen)//maximum generation reached
		{
			if (logger) logger->Log("MaxGen value reached, terminating - generation " + to_string(generation) + ".\n", SUBEVENT);
			terminationReason = "maxGen value reached, final fitness: " + to_string(bestFitness);
			success = false;
			break;
		}
		if (funEvals >= maxFunEvals)//maximum function evaluations exhausted
		{
			if (logger) logger->Log("MaxFunEvals value reached, terminating - generation " + to_string(generation) + ".\n", SUBEVENT);
			terminationReason = "maxFunEvals value reached, final fitness: " + to_string(bestFitness);
			success = false;
			break;
		}
		if (historyConstant)//no entity improved last (historySize) generations
		{
			if (logger) logger->Log("historyConstant value reached, terminating - generation " + to_string(generation) + ".\n", SUBEVENT);
			terminationReason = "historyConstant value reached, final fitness: " + to_string(bestFitness);
			success = false;
			break;
		}
	}//generation cycle end
	if (logPoints) visitedPoints.push_back(visitedPointsThisRun);
	return bestEntity;
}//optimize function end

std::vector<double> PatternSearch::optimize(std::function<double(std::vector<double>)> f, Logger* logger, IPlot1D* plt)
{
	//if (logger) logger->Log    ">> Optimization started (pattern search)"  ;

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

		vector<vector<double>> visitedPointsThisRun;
		vector<vector<double>> visitedPointsMainThisRun;
		int funEvalsThisRun = 0;
		//initialize vectors
		double step = initialStep;
		vector<double> mainPoint = mainPointsInitial[run];
		double mainPointFitness = f(mainPoint);
		if (logPoints) visitedPointsThisRun.push_back(mainPoint);
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

					if (logPoints) visitedPointsThisRun.push_back(pattern[dim][pm]);

					//select best pattern vertex and replace
					if (patternFitness[dim][pm] < mainPointFitness)
					{
						mainPoint = pattern[dim][pm];
						mainPointFitness = patternFitness[dim][pm];
						smallerStep = false;
						//if (logger) logger->Log  "> run "  run  " current best entity fitness: "  patternFitness[dim][pm]  ;
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
								if (logPoints) visitedPointsThisRun.push_back(testPoint);
								if (testPointFitness < mainPointFitness)
								{
									mainPoint = testPoint;
									mainPointFitness = testPointFitness;
									//if (logger) logger->Log  "> run "  run  " - exploitation "  exploitCnt  " just improved the fitness: "  testPointFitness  ;
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
					//if (logger) logger->Log  "> minStep value reached, terminating - generation "  generation  "."  ;
					success = false;
					terminationReason = "minStep value reached, final fitness: " + to_string(mainPointFitness);
				}
				break;
			}
			if (mainPointFitness < optimalFitness)
			{
				#pragma omp critical
				{
					//if (logger) logger->Log  "> optimalFitness value reached, terminating - generation "  generation  "."  ;
					success = true;
					terminationReason = "optimalFitness value reached, final fitness: " + to_string(mainPointFitness);
				}
				break;
			}
			if (generation == maxGen)
			{
				#pragma omp critical
				{
					//if (logger) logger->Log  "> maxGen value reached, terminating - generation "  generation  "."  ;
					success = false;
					terminationReason = "maxGen value reached, final fitness: " + to_string(mainPointFitness);
				}
				break;
			}
			if ((funEvals >= maxFunEvals) || (funEvalsThisRun >= maxFunEvals))
			{
				#pragma omp critical
				{
					//if (logger) logger->Log  "MaxFunEvals value reached, terminating - generation "  generation  "."  ;
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
			if (logPoints) visitedPoints.push_back(visitedPointsThisRun);
			//if (logger) logger->Log  "> run "  run  ": ";
			if (mainPointFitness < topPointFitness)
			{
				topPoint = mainPoint;
				topPointFitness = mainPointFitness;
				//if (logger) showEntity(topPoint, topPointFitness, "current multistart best", true, true);
				if (topPointFitness < optimalFitness) flag = true;//dont do other runs, fitness goal reached
			}
			else
			{
				//if (logger) logger->Log  "- run has ended with no improvement, fitness: "  mainPointFitness  ;
			}
		}
	}//multistart end
	return topPoint;
}//opt end

#ifdef OPTIMIZE_WITH_CV
using namespace cv;

Mat drawFunc2D(std::function<double(vector<double>)> f, double xmin, double xmax, double ymin, double ymax, int stepsX, int stepsY)
{
	Mat resultMat = Mat::zeros(stepsY, stepsX, CV_32F);
	int progress = 0;
	bool OpenMPparallelism = true;

	RunInParallelOrInSerial(0, resultMat.rows, true, OpenMPparallelism, [&](int r)//WS needs windows parallelism affinity to utilize all 4 NUMA cpu nodes
	{
		for (int c = 0; c < resultMat.cols; c++)
		{
			double x = xmin + (xmax - xmin) / (stepsX - 1) * c;
			double y = ymax - (ymax - ymin) / (stepsY - 1) * r;
			resultMat.at<float>(r, c) = f(vector<double>{x, y});
		}
		#pragma omp critical
		{
			progress++;
			cout << (double)progress / resultMat.rows * 100 << "% done." << endl;
		}
	});
	normalize(resultMat, resultMat, 0, 255, CV_MINMAX);
	return resultMat;
}

Mat drawPath2D(Mat funcLandscape, vector<vector<vector<double>>> points, double xmin, double xmax, double ymin, double ymax, int stepsX, int stepsY, double stretchFactorX, double stretchFactorY, bool drawArrows)
{
	Mat resultMat = funcLandscape.clone();
	for (int run = 0; run < points.size(); run++)
	{
		auto colorThisRun = cv::Scalar(rand() % 256, rand() % 256, rand() % 256);
		for (int i = 0; i < points[run].size(); i++)
		{
			int col1 = (points[run][i][0] - xmin) / (xmax - xmin)*(stepsX - 1);
			int row1 = stepsY - (points[run][i][1] - ymin) / (ymax - ymin)*(stepsY - 1);
			cv::Point pointik1(col1, row1);
			if (i == 0)
			{
				if (drawArrows) drawPoint2D(resultMat, pointik1, stretchFactorX, stretchFactorY, Scalar(255, 0, 0));//start of path-B
			}
			else if (i == points[run].size() - 1)
			{
				if (drawArrows) drawPoint2D(resultMat, pointik1, stretchFactorX, stretchFactorY, Scalar(0, 255, 0));//end of path-G
			}
			else
				drawPoint2D(resultMat, pointik1, stretchFactorX, stretchFactorY, colorThisRun);

			if (drawArrows)
			{
				if (i < (points[run].size() - 1))
				{
					int col2 = (points[run][i + 1][0] - xmin) / (xmax - xmin)*(stepsX - 1);
					int row2 = stepsY - (points[run][i + 1][1] - ymin) / (ymax - ymin)*(stepsY - 1);
					cv::Point pointik2(col2, row2);
					drawArrow2D(resultMat, pointik1, pointik2, stretchFactorX, stretchFactorY, colorThisRun);
				}
			}
		}
	}
	return resultMat;
}

std::vector<double> drawFuncLandscapeAndOptimize2D(std::function<double(vector<double>)> f, vector<double> lowerBounds, vector<double> upperBounds, vector<int> steps, Evolution Evo, PatternSearch Pat, bool logLandscapeOpt, bool optPat, bool optEvo, double quantileB, double quantileT, Mat* landscape)
{
	Mat optimizedFuncLandscapeCLR = Mat::zeros(steps[0], steps[1], CV_32F);
	cv::Point2i minloc, maxloc;
	vector<double> minlocArg(2, 0);
	double stretchFactorX, stretchFactorY;
	cout << "Drawing the function landscape... " << endl;
	Mat optimizedFuncLandscapeRAW = drawFunc2D(f, lowerBounds[0], upperBounds[0], lowerBounds[1], upperBounds[1], steps[0], steps[1]);
	if (landscape) *landscape = optimizedFuncLandscapeRAW;
	optimizedFuncLandscapeCLR = applyColorMapZdeny(optimizedFuncLandscapeRAW, quantileB, quantileT);
	Mat optimizedFuncLandscapeRAWlog = optimizedFuncLandscapeRAW.clone() + Scalar::all(1.);
	log(optimizedFuncLandscapeRAWlog, optimizedFuncLandscapeRAWlog);
	normalize(optimizedFuncLandscapeRAW, optimizedFuncLandscapeRAW, 0, 255, CV_MINMAX);
	normalize(optimizedFuncLandscapeRAWlog, optimizedFuncLandscapeRAWlog, 0, 255, CV_MINMAX);
	Mat optimizedFuncLandscapeCLRlog = applyColorMapZdeny(optimizedFuncLandscapeRAWlog, quantileB, quantileT);//log or not

	double minim, maxim;
	minMaxLoc(optimizedFuncLandscapeRAW, &minim, &maxim, &minloc, &maxloc);
	minlocArg[0] = lowerBounds[0] + (upperBounds[0] - lowerBounds[0]) / (steps[0] - 1) * minloc.x;
	minlocArg[1] = upperBounds[1] - (upperBounds[1] - lowerBounds[1]) / (steps[1] - 1) * minloc.y;

	int stretchSize = 1001;//odd
	stretchFactorX = (double)stretchSize / optimizedFuncLandscapeRAW.cols;
	stretchFactorY = (double)stretchSize / optimizedFuncLandscapeRAW.rows;
	resize(optimizedFuncLandscapeCLR, optimizedFuncLandscapeCLR, cv::Size(stretchSize, stretchSize), 0, 0, INTER_NEAREST);
	resize(optimizedFuncLandscapeCLRlog, optimizedFuncLandscapeCLRlog, cv::Size(stretchSize, stretchSize), 0, 0, INTER_NEAREST);
	drawPoint2D(optimizedFuncLandscapeCLR, minloc, stretchFactorX, stretchFactorY, Scalar(0, 0, 255));
	drawPoint2D(optimizedFuncLandscapeCLRlog, minloc, stretchFactorX, stretchFactorY, Scalar(0, 0, 255));
	//showimg(optimizedFuncLandscapeCLR, "optimizedFuncLandscape");
	//showimg(optimizedFuncLandscapeCLRlog, "optimizedFuncLandscape-log");
	//showEntity(minlocArg, f(minlocArg), "Result - BRUTE", true);
	cout << "done." << endl;

	if (1)
	{
		if (logLandscapeOpt) optimizedFuncLandscapeCLR = optimizedFuncLandscapeCLRlog;

		Evo.logPoints = true;//needed for pretty pictures - main directed path

		vector<double> resultPat = zerovect(Evo.N), resultEvo = zerovect(Evo.N);
		if (optPat)
		{
			resultPat = Pat.optimize(f);
		}
		if (optEvo)
		{
			resultEvo = Evo.optimize(f);
		}

		//OUTPUT
		cout << endl;
		if (optPat) cout << "Termination reason - PAT: " << Pat.terminationReason << endl;
		if (optEvo) cout << "Termination reason - EVO: " << Evo.terminationReason << endl;
		if (optPat) cout << "Function evals - PAT: " << Pat.funEvals << " (multistartCnt: " << Pat.multistartCnt << ")" << endl;
		if (optEvo) cout << "Function evals - EVO: " << Evo.funEvals << endl;
		//if (optPat) showEntity(resultPat, f(resultPat), "Result - PAT", true);
		//if (optEvo) showEntity(resultEvo, f(resultEvo), "Result - EVO", true);
		//showEntity(minlocArg, f(minlocArg), "Result - BRUTE", true);
		Mat optimizedFuncLandscapeWithPathPAT = optimizedFuncLandscapeCLR, optimizedFuncLandscapeWithPathEVO = optimizedFuncLandscapeCLR;
		if (optPat) optimizedFuncLandscapeWithPathPAT = drawPath2D(optimizedFuncLandscapeWithPathPAT, Pat.visitedPoints, lowerBounds[0], upperBounds[0], lowerBounds[1], upperBounds[1], steps[0], steps[1], stretchFactorX, stretchFactorY, 0);
		if (optEvo) optimizedFuncLandscapeWithPathEVO = drawPath2D(optimizedFuncLandscapeWithPathEVO, Evo.visitedPoints, lowerBounds[0], upperBounds[0], lowerBounds[1], upperBounds[1], steps[0], steps[1], stretchFactorX, stretchFactorY, 0);
		if (optPat) optimizedFuncLandscapeWithPathPAT = drawPath2D(optimizedFuncLandscapeWithPathPAT, Pat.visitedPoints, lowerBounds[0], upperBounds[0], lowerBounds[1], upperBounds[1], steps[0], steps[1], stretchFactorX, stretchFactorY, 1);
		if (optEvo) optimizedFuncLandscapeWithPathEVO = drawPath2D(optimizedFuncLandscapeWithPathEVO, Evo.visitedPoints, lowerBounds[0], upperBounds[0], lowerBounds[1], upperBounds[1], steps[0], steps[1], stretchFactorX, stretchFactorY, 1);

		if (optPat) drawPoint2D(optimizedFuncLandscapeWithPathPAT, minloc, stretchFactorX, stretchFactorY, Scalar(0, 0, 255));
		if (optEvo) drawPoint2D(optimizedFuncLandscapeWithPathEVO, minloc, stretchFactorX, stretchFactorY, Scalar(0, 0, 255));
		if (optPat) showimg(optimizedFuncLandscapeWithPathPAT, "optimizedFuncLandscapeWithPath - PAT");
		if (optEvo) showimg(optimizedFuncLandscapeWithPathEVO, "optimizedFuncLandscapeWithPath - EVO");
	}
	return minlocArg;
}

void optimizeWithLandscapeDebug()
{
	//normal
	std::function<double(vector<double>)> f = optimizationTestFunctions::Rosenbrock;
	vector<double> lowerBounds{ -4.5, -4.5 };
	vector<double> upperBounds{ 4.5, 4.5 };//odd
	vector<int> steps{ 501, 501 };//odd

	/*
	//meta Evo
	std::function<double(vector<double>)> f = metaOptFuncEvo;
	vector<double> lowerBounds{ 15, 0.7, 0.5, 0.5 };
	vector<double> upperBounds{ 15, 0.7, 0.5, 0.5 };//odd
	vector<int> steps{ (int)(2 * pmranges[0] + 1), (int)(2 * pmranges[0] + 1), (int)(2 * pmranges[0] + 1), 3 };//odd
	*/

	Evolution Evo(lowerBounds.size());
	PatternSearch Pat(lowerBounds.size());

	Evo.lowerBounds = lowerBounds;
	Evo.upperBounds = upperBounds;
	Evo.optimalFitness = 1e-4;
	Evo.NP = 50;

	Pat.lowerBounds = lowerBounds;
	Pat.upperBounds = upperBounds;
	Pat.optimalFitness = 1e-4;
	Pat.multistartMaxCnt = 3;

	auto Result = drawFuncLandscapeAndOptimize2D(f, lowerBounds, upperBounds, steps, Evo, Pat, 1, 1, 1, 0, 1);

	cout << "> Done broski." << endl;
}

#endif
