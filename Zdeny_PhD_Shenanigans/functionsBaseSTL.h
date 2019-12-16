#pragma once
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <string>
#include <chrono>
#include <numeric>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <filesystem>
#include <queue>
#include <functional>
#include <vector>

using namespace std;

constexpr double PI = 3.1415926535897932384626433;
constexpr double E = 2.7182818284590452353602874;
constexpr int dummyInt = 12345;
constexpr double dummyDouble = 12345.6789;

const std::string delimiter = ",";
const std::string consoleDivider = "<<================================================================================>>";

struct Timerr//benchmarking struct
{
	std::chrono::time_point<std::chrono::high_resolution_clock> startTimepoint;
	std::string name;
	inline Timerr(std::string name) : name(name)
	{
		startTimepoint = std::chrono::high_resolution_clock::now();
	}

	~Timerr()
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> endTimepoint = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTimepoint).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
		auto duration = end - start;
		double ms = duration * 1e-3;
		cout << "<timer - " << name << "> (" << ms << " ms)" << endl;
	}
};

enum class Norm : char { L1, L2 };

inline std::vector<double> zerovect(int N, double value = 0)
{
	return std::vector<double>(N, value);
}

inline std::vector<std::vector<double>> zerovect2(int N, int M, double value = 0)
{
	return std::vector<std::vector<double>>(N, zerovect(M, value));
}

inline std::vector<std::vector<std::vector<double>>> zerovect3(int N, int M, int O, double value = 0)
{
	return std::vector<std::vector<std::vector<double>>>(N, zerovect2(M, O, value));
}

inline std::vector<std::vector<std::vector<std::vector<double>>>> zerovect4(int N, int M, int O, int P, double value = 0)
{
	return std::vector<std::vector<std::vector<std::vector<double>>>>(N, zerovect3(M, O, P, value));
}

inline std::vector<std::vector<std::vector<std::vector<std::vector<double>>>>> zerovect5(int N, int M, int O, int P, int Q, double value = 0)
{
	return std::vector<std::vector<std::vector<std::vector<std::vector<double>>>>>(N, zerovect4(M, O, P, Q, value));
}

template <typename T>
inline ostream& operator<<(ostream& out, std::vector<T>& vec)
{
	out << "[";
	for (int i = 0; i < vec.size(); i++)
	{
		out << vec[i];
		if (i < vec.size() - 1) out << ", ";
	}
	out << "]";
	return out;
}

inline ostream& operator<<(ostream& out, std::vector<std::string>& vec)
{
	for (int i = 0; i < vec.size(); i++)
	{
		out << vec[i] << endl;
	}
	return out;
}

template <typename T>
inline T sqr(T x)
{
	return x * x;
}

template <typename T>
inline double mean(std::vector<T>& vec)
{
	double mean = 0;
	for (auto& x : vec)
		mean += x;
	return mean / vec.size();
}

inline double mean2(std::vector<std::vector<double>>& vec)
{
	double mean = 0;
	for (auto& row : vec)
		for (auto& x : row)
			mean += x;
	return mean / vec.size() / vec[0].size();
}

template <typename T>
inline double stdev(std::vector<T>& vec)
{
	double m = mean(vec);
	double stdev = 0;
	for (auto& x : vec)
		stdev += sqr(x - m);
	stdev /= vec.size();
	return sqrt(stdev);
}

template <typename T>
inline std::vector<T> operator+(std::vector<T>& vec1, std::vector<T>& vec2)
{
	std::vector<T> result = vec1;
	for (int i = 0; i < vec1.size(); i++)
		result[i] = vec1[i] + vec2[i];
	return result;
}

template <typename T>
inline std::vector<T> operator-(std::vector<T>& vec1, std::vector<T>& vec2)
{
	std::vector<T> result = vec1;
	for (int i = 0; i < vec1.size(); i++)
		result[i] = vec1[i] - vec2[i];
	return result;
}

template <typename T>
inline std::vector<T>& operator+=(std::vector<T>& vec1, std::vector<T>& vec2)
{
	for (int i = 0; i < vec1.size(); i++)
		vec1[i] += vec2[i];
	return vec1;
}

template <typename T>
inline std::vector<T>& operator-=(std::vector<T>& vec1, std::vector<T>& vec2)
{
	for (int i = 0; i < vec1.size(); i++)
		vec1[i] -= vec2[i];
	return vec1;
}

template <typename T>
inline std::vector<T> operator*(double val, std::vector<T>& vec)
{
	std::vector<T> result = vec;
	for (int i = 0; i < vec.size(); i++)
		result[i] = val * vec[i];
	return result;
}

template <typename T>
inline double median(std::vector<T>& vec)
{
	//function changes the vec order, watch out
	std::sort(vec.begin(), vec.end());
	return vec[vec.size() / 2];
}

template <typename Func>
inline void RunInParallelOrInSerialZdeny(int zeroIndex, int maxIndex, bool isParallelRunEnabled, bool openMP, Func fnc)
{
	if (isParallelRunEnabled)
	{
		if (openMP)//OpenMP parallelism
		{
#pragma omp parallel for
			for (int i = zeroIndex; i < maxIndex; i++)
			{
				fnc(i);
			}
		}
		else//Windows parallelism
		{
			//Concurrency::parallel_for(zeroIndex, maxIndex, fnc);
		}
	}
	else//serial
	{
		for (int i = zeroIndex; i < maxIndex; i++)
		{
			fnc(i);
		}
	}
}

inline bool vectorLess(std::vector<double>& left, std::vector<double>& right)
{
	double L = 0, R = 0;
	for (int i = 0; i < left.size(); i++)
	{
		L += abs(left[i]);
		R += abs(right[i]);
	}
	return L < R;
}

inline double clamp(double x, double clampMin, double clampMax)
{
	return min(max(x, clampMin), clampMax);
}

inline double clampSmooth(double x_new, double x_prev, double clampMin, double clampMax)
{
	return x_new < clampMin ? (x_prev + clampMin) / 2 : x_new > clampMax ? (x_prev + clampMax) / 2 : x_new;
}

template <typename T>
inline T vectorMax(std::vector<T>& input)
{
	T vectmax = input[0];
	for (int i = 0; i < input.size(); i++)
	{
		if (input[i] > vectmax)
			vectmax = input[i];
	}
	return vectmax;
}

template <typename T>
inline T vectorMin(std::vector<T>& input)
{
	T vectmin = input[0];
	for (int i = 0; i < input.size(); i++)
	{
		if (input[i] < vectmin)
			vectmin = input[i];
	}
	return vectmin;
}

template <typename T>
inline T arrayMax(T* input, unsigned size)
{
	T arrmax = input[0];
	for (int i = 0; i < size; i++)
	{
		if (input[i] > arrmax)
			arrmax = input[i];
	}
	return arrmax;
}

template <typename T>
inline T arrayMin(T* input, unsigned size)
{
	T arrmin = input[0];
	for (int i = 0; i < size; i++)
	{
		if (input[i] < arrmin)
			arrmin = input[i];
	}
	return arrmin;
}

inline std::string currentDateTime()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	strftime(buf, sizeof(buf), "%Y-%b%d-%H.%M.%S", &tstruct);
	return buf;
}

inline std::string currentTime()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	strftime(buf, sizeof(buf), "%H:%M:%S", &tstruct);
	return buf;
}

inline double speedTest(double x)
{
	return pow(asin(x*x) + floor(x)*(x - 123.4) + PI * cos(atan(1. / x)), 0.123456);
}

inline double gaussian1D(double x, double amp, double mu, double sigma)
{
	return amp*exp(-0.5*pow((x - mu) / sigma, 2));
}

inline void mkdir(std::string path)
{
	std::experimental::filesystem::create_directory(path);
}

constexpr int factorial(int n)
{
	return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
}

inline double randInRange(double min_ = 0, double max_ = 1)
{
	return min_ + (double)rand() / RAND_MAX * (max_ - min_);
}

inline void linreg(int n, const std::vector<double>&x, const std::vector<double>&y, double& k, double& q)
{
	double sumx = 0.;                        /* sum of x                      */
	double sumx2 = 0.;                       /* sum of x**2                   */
	double sumxy = 0.;                       /* sum of x * y                  */
	double sumy = 0.;                        /* sum of y                      */
	double sumy2 = 0.;                       /* sum of y**2                   */

	for (int i = 0; i < n; i++)
	{
		sumx += x[i];
		sumx2 += sqr(x[i]);
		sumxy += x[i] * y[i];
		sumy += y[i];
		sumy2 += sqr(y[i]);
	}

	double denom = (n * sumx2 - sqr(sumx));

	if (denom)
	{
		k = (n * sumxy - sumx * sumy) / denom;
		q = (sumy * sumx2 - sumx * sumxy) / denom;
	}
	else
	{
		k = 0;
		q = 0;
	}
}

inline double linregPosition(int n, const std::vector<double>&x, const std::vector<double>&y, double x_)
{
	double k, q;
	linreg(n, x, y, k, q);
	return k * x_ + q;
}