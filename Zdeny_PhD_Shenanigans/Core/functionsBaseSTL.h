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

const std::string delimiter = ",";

struct Timerr//benchmarking struct
{
	std::chrono::time_point<std::chrono::high_resolution_clock> startTimepoint;
	std::string name;
	Timerr( std::string name ) : name( name )
	{
		startTimepoint = std::chrono::high_resolution_clock::now();
	}

	~Timerr()
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> endTimepoint = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>( startTimepoint ).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>( endTimepoint ).time_since_epoch().count();
		cout << "<<" << name << ">> took " << double( end - start ) << " mics" << endl;
	}
};

struct Timerrr//benchmarking struct
{
	double startTimepoint;
	std::string name;
	clock_t t;
	Timerrr( std::string name ) : name( name )
	{
		t = clock();
	}

	~Timerrr()
	{
		t = clock() - t;
		cout << "<<" << name << ">> took " << std::setprecision( 20 ) << t << " ms" << endl;
	}
};

inline double randunit()
{
	return ( double )rand() / RAND_MAX;
}

inline double rand01()
{
	return ( double )rand() / RAND_MAX;
}

inline double rand11()
{
	return 2.0 * rand01() - 1.0;
}

inline double clamp( double x, double clampMin, double clampMax )
{
	return min( max( x, clampMin ), clampMax );
}

inline double clampSmooth( double x_new, double x_prev, double clampMin, double clampMax )
{
	return x_new < clampMin ? ( x_prev + clampMin ) / 2 : x_new > clampMax ? ( x_prev + clampMax ) / 2 : x_new;
}

template <typename T = double>
inline auto zerovect( int N, T value = 0. )
{
	return std::vector<T>( N, value );
}

template <typename T = double>
inline auto zerovect2( int N, int M, T value = 0. )
{
	return std::vector<std::vector<T>>( N, zerovect( M, value ) );
}

template <typename T = double>
inline auto zerovect3( int N, int M, int O, T value = 0. )
{
	return std::vector<std::vector<std::vector<T>>>( N, zerovect2( M, O, value ) );
}

template <typename T = double>
inline auto zerovect4( int N, int M, int O, int P, T value = 0. )
{
	return std::vector<std::vector<std::vector<std::vector<T>>>>( N, zerovect3( M, O, P, value ) );
}

template <typename T>
inline ostream &operator<<( ostream &out, std::vector<T> &vec )
{
	out << "[";
	for ( int i = 0; i < vec.size(); i++ )
	{
		out << vec[i];
		if ( i < vec.size() - 1 ) out << ", ";
	}
	out << "]";
	return out;
}

template <typename T>
inline ostream &operator<<( ostream &out, std::vector<std::vector<T>> &vec )
{
	out << endl;
	for ( int r = 0; r < vec.size(); r++ )
	{
		out << "[";
		for ( int c = 0; c < vec[r].size(); c++ )
		{
			out << vec[r][c];
			if ( c < vec[r].size() - 1 ) out << ", ";
		}
		out << "]";
		out << endl;
	}
	return out;
}

inline ostream &operator<<( ostream &out, std::vector<std::string> &vec )
{
	for ( int i = 0; i < vec.size(); i++ )
	{
		out << vec[i] << endl;
	}
	return out;
}

template <typename T>
inline constexpr T sqr( T x )
{
	return x * x;
}

template <typename T>
inline double mean( const std::vector<T> &vec )
{
	double mean = 0;
	for ( auto &x : vec )
		mean += x;
	return mean / vec.size();
}

template <typename T>
inline double mean( const std::vector<std::vector<T>> &vec )
{
	double mean = 0;
	for ( auto &row : vec )
		for ( auto &x : row )
			mean += x;
	return mean / vec.size() / vec[0].size();
}

template <typename T>
inline double stdev( const std::vector<T> &vec )
{
	double m = mean( vec );
	double stdev = 0;
	for ( auto &x : vec )
		stdev += sqr( x - m );
	stdev /= vec.size();
	return sqrt( stdev );
}

template <typename T>
inline double stdevs( const std::vector<T> &vec )
{
	double m = mean( vec );
	double stdev = 0;
	for ( auto &x : vec )
		stdev += sqr( x - m );
	stdev /= ( vec.size() - 1 );
	return sqrt( stdev );
}

template <typename T>
inline std::vector<T> operator+( const std::vector<T> &vec1, const std::vector<T> &vec2 )
{
	std::vector<T> result = vec1;
	for ( int i = 0; i < vec1.size(); i++ )
		result[i] = vec1[i] + vec2[i];
	return result;
}

template <typename T>
inline std::vector<T> operator-( const std::vector<T> &vec1, const std::vector<T> &vec2 )
{
	std::vector<T> result = vec1;
	for ( int i = 0; i < vec1.size(); i++ )
		result[i] = vec1[i] - vec2[i];
	return result;
}

template <typename T>
inline std::vector<T> &operator+=( std::vector<T> &vec1, const std::vector<T> &vec2 )
{
	for ( int i = 0; i < vec1.size(); i++ )
		vec1[i] += vec2[i];
	return vec1;
}

template <typename T>
inline std::vector<T> &operator-=( std::vector<T> &vec1, const std::vector<T> &vec2 )
{
	for ( int i = 0; i < vec1.size(); i++ )
		vec1[i] -= vec2[i];
	return vec1;
}

template <typename T>
inline std::vector<T> operator*( double val, const std::vector<T> &vec )
{
	std::vector<T> result = vec;
	for ( int i = 0; i < vec.size(); i++ )
		result[i] = val * vec[i];
	return result;
}

template <typename T>
inline std::vector<T> abs( const std::vector<T> &vec )
{
	auto result = vec;
	for ( int i = 0; i < vec.size(); i++ )
		result[i] = abs( vec[i] );
	return result;
}

template <typename T>
inline double median( const std::vector<T> &vec )
{
	auto result = vec;
	std::sort( result.begin(), result.end() );
	return result[result.size() / 2];
}

inline bool vectorLess( std::vector<double> &left, std::vector<double> &right )
{
	double L = 0, R = 0;
	for ( int i = 0; i < left.size(); i++ )
	{
		L += abs( left[i] );
		R += abs( right[i] );
	}
	return L < R;
}

template <typename T>
inline T vectorMax( const std::vector<T> &input )
{
	T vectmax = input[0];
	for ( int i = 0; i < input.size(); i++ )
	{
		if ( input[i] > vectmax )
			vectmax = input[i];
	}
	return vectmax;
}

template <typename T>
inline T vectorMin( const std::vector<T> &input )
{
	T vectmin = input[0];
	for ( int i = 0; i < input.size(); i++ )
	{
		if ( input[i] < vectmin )
			vectmin = input[i];
	}
	return vectmin;
}

template <typename T>
inline T arrayMax( T *input, unsigned size )
{
	T arrmax = input[0];
	for ( int i = 0; i < size; i++ )
	{
		if ( input[i] > arrmax )
			arrmax = input[i];
	}
	return arrmax;
}

template <typename T>
inline T arrayMin( T *input, unsigned size )
{
	T arrmin = input[0];
	for ( int i = 0; i < size; i++ )
	{
		if ( input[i] < arrmin )
			arrmin = input[i];
	}
	return arrmin;
}

inline std::string currentDateTime()
{
	time_t     now = time( 0 );
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime( &now );
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	strftime( buf, sizeof( buf ), "%Y-%b%d-%H.%M.%S", &tstruct );
	return buf;
}

inline std::string currentTime()
{
	time_t     now = time( 0 );
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime( &now );
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	strftime( buf, sizeof( buf ), "%H:%M:%S", &tstruct );
	return buf;
}

inline double speedTest( double x )
{
	return pow( asin( x * x ) + floor( x ) * ( x - 123.4 ) + 3.14 * cos( atan( 1. / x ) ), 0.123456 );
}

inline double gaussian1D( double x, double amp, double mu, double sigma )
{
	return amp * exp( -0.5 * pow( ( x - mu ) / sigma, 2 ) );
}

constexpr int factorial( int n )
{
	return ( n == 1 || n == 0 ) ? 1 : factorial( n - 1 ) * n;
}

inline double randInRange( double min_ = 0, double max_ = 1 )
{
	return min_ + ( double )rand() / RAND_MAX * ( max_ - min_ );
}

inline void linreg( int n, const std::vector<double> &x, const std::vector<double> &y, double &k, double &q )
{
	double sumx = 0.;                        /* sum of x                      */
	double sumx2 = 0.;                       /* sum of x**2                   */
	double sumxy = 0.;                       /* sum of x * y                  */
	double sumy = 0.;                        /* sum of y                      */
	double sumy2 = 0.;                       /* sum of y**2                   */

	for ( int i = 0; i < n; i++ )
	{
		sumx += x[i];
		sumx2 += sqr( x[i] );
		sumxy += x[i] * y[i];
		sumy += y[i];
		sumy2 += sqr( y[i] );
	}

	double denom = ( n * sumx2 - sqr( sumx ) );

	if ( denom )
	{
		k = ( n * sumxy - sumx * sumy ) / denom;
		q = ( sumy * sumx2 - sumx * sumxy ) / denom;
	}
	else
	{
		k = 0;
		q = 0;
	}
}

inline double linregPosition( int n, const std::vector<double> &x, const std::vector<double> &y, double x_ )
{
	double k, q;
	linreg( n, x, y, k, q );
	return k * x_ + q;
}

inline double averageVectorDistance( std::vector<double> &vec1, std::vector<double> &vec2, std::vector<double> &boundsRange )
{
	double result = 0;
	for ( int i = 0; i < vec1.size(); i++ )
	{
		result += abs( vec1[i] - vec2[i] ) / boundsRange[i]; //normalize -> 0 to 1
	}
	result /= vec1.size();//coordinate average
	return result;
}

inline bool isDistinct( int inpindex, std::vector<int> &indices, int currindex )
{
	bool isdist = true;
	for ( auto &idx : indices )
	{
		if ( inpindex == idx || inpindex == currindex )
			isdist = false;
	}
	return isdist;
}

inline void exportToMATLAB( const std::vector<double> &Ydata, double xmin, double xmax, std::string path )
{
	std::ofstream listing( path, std::ios::out | std::ios::trunc );
	listing << xmin << "," << xmax << endl;
	for ( auto &y : Ydata )
		listing << y << endl;
}

inline void exportToMATLAB( const std::vector<std::vector<double>> &Zdata, double xmin, double xmax, double ymin, double ymax, std::string path )
{
	std::ofstream listing( path, std::ios::out | std::ios::trunc );
	listing << xmin << "," << xmax << "," << ymin << "," << ymax << endl;
	for ( int r = 0; r < Zdata.size(); r++ )
		for ( int c = 0; c < Zdata[0].size(); c++ )
			listing << Zdata[r][c] << endl;
}

inline void makeDir( std::string path, std::string dirname )
{
	std::experimental::filesystem::create_directory( path + "//" + dirname );
}

inline std::vector<double> iota( int first, int size )
{
	std::vector<double> vec( size );
	for ( int i = 0; i < vec.size(); i++ )
		vec[i] = first + i;
	return vec;
}

inline std::string operator+( const std::string &str, const int val )
{
	return str + to_string( val );
}

inline std::string operator+( const int val, const std::string &str )
{
	return to_string( val ) + str;
}

inline void filterMedian( std::vector<double> &vec, int size )
{
	std::vector<double> vecMedian = vec;
	std::vector<double> med;
	med.reserve( size );

	for ( int i = 0; i < vec.size(); i++ )
	{
		med.clear();
		for ( int m = 0; m < size; m++ )
		{
			int idx = i - size / 2 + m;

			if ( idx < 0 )
				continue;
			if ( idx == vec.size() )
				break;

			med.emplace_back( vec[idx] );
		}

		vecMedian[i] = median( med );
	}
	vec = vecMedian;
}

inline void filterMovavg( std::vector<double> &vec, int size )
{
	std::vector<double> vecMovavg = vec;
	double movavg;
	int movavgcnt;

	for ( int i = 0; i < vec.size(); i++ )
	{
		movavg = 0;
		movavgcnt = 0;
		for ( int m = 0; m < size; m++ )
		{
			int idx = i - size / 2 + m;

			if ( idx < 0 )
				continue;
			if ( idx == vec.size() )
				break;

			movavg += vec[idx];
			movavgcnt += 1;
		}
		movavg /= ( double )movavgcnt;
		vecMovavg[i] = movavg;
	}
	vec = vecMovavg;
}

inline double toDegrees( double rad )
{
	if ( rad < 0 )
	{
		return ( rad + Constants::TwoPi ) * Constants::Rad;
	}
	else
	{
		return rad * Constants::Rad;
	}
}

inline double toRadians( double deg )
{
	return deg / Constants::Rad;
}

inline std::vector<double> toDegrees( const std::vector<double> &vecrad )
{
	auto vecdeg = vecrad;
	for ( int i = 0; i < vecrad.size(); i++ )
	{
		vecdeg[i] = toDegrees( vecrad[i] );
	}
	return vecdeg;
}

inline std::vector<double> toRadians( const std::vector<double> &vecdeg )
{
	auto vecrad = vecdeg;
	for ( int i = 0; i < vecdeg.size(); i++ )
	{
		vecrad[i] = toRadians( vecdeg[i] );
	}
	return vecrad;
}

inline std::string to_stringp( double val, int prec )
{
	std::string vals = to_string( val );
	return vals.substr( 0, vals.find( "." ) + prec + 1 );
}

inline std::vector<double> removeQuantileOutliers( const std::vector<double> &vec, double quanB, double quanT )
{
	auto out = vec;
	std::sort( out.begin(), out.end() );
	return std::vector<double>( out.begin() + ( int )( quanB * ( out.size() - 1 ) ), out.begin() + ( int )( quanT * ( out.size() - 1 ) ) );
}

inline double getQuantile( const std::vector<double> &vec, double quan )
{
	std::vector<double> out = vec;
	std::sort( out.begin(), out.end() );
	return out[( int )( quan * ( out.size() - 1 ) )];
}

inline double getQuantile( const std::vector<std::vector<double>> &vec, double quan )
{
	std::vector<double> out;
	out.reserve( vec.size()*vec[0].size() );
	for ( int i = 0; i < vec.size(); i++ )
	{
		for ( double x : vec[i] )
		{
			out.push_back( x );
		}
	}
	std::sort( out.begin(), out.end() );
	return out[( int )( quan * ( out.size() - 1 ) )];
}

inline std::vector<double> getStandardErrorsOfTheMeanHorizontal( const std::vector<std::vector<double>> &vec )
{
	std::vector<double> errors( vec.size() );

	for ( int i = 0; i < vec.size(); i++ )
	{
		double m = mean( vec[i] );
		double s = stdevs( vec[i] );
		double n = vec[i].size();
		double e = s / sqrt( n );
		errors[i] = e;
	}
	return errors;
}

inline std::vector<double> getStandardErrorsOfTheMeanVertical( const std::vector<std::vector<double>> &vec )
{
	std::vector<double> errors( vec[0].size() );

	for ( int i = 0; i < vec[0].size(); i++ )
	{
		std::vector<double> v( vec.size() );
		for ( int j = 0; j < vec.size(); j++ )
			v[j] = vec[j][i];

		double m = mean( v );
		double s = stdevs( v );
		double n = v.size();
		double e = s / sqrt( n );
		errors[i] = e;
	}
	return errors;
}

inline std::vector<double> getStandardDeviationsVertical( const std::vector<std::vector<double>> &vec )
{
	std::vector<double> stddevs( vec[0].size() );

	for ( int i = 0; i < vec[0].size(); i++ )
	{
		std::vector<double> v( vec.size() );
		for ( int j = 0; j < vec.size(); j++ )
			v[j] = vec[j][i];

		stddevs[i] = stdevs( v );
	}
	return stddevs;
}