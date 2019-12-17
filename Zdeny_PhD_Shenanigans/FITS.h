#pragma once
#include "functionsBaseSTL.h"
#include "functionsBaseCV.h"
#include "filtering.h"

using namespace std;
using namespace cv;

//.fits parameters
constexpr int lineBYTEcnt = 80;
constexpr int linesMultiplier = 36;
constexpr int timeDelta = 45;
enum class fitsType : char { HMI, AIA, SECCHI };

struct fitsParams
{
	double fitsMidX = 0;
	double fitsMidY = 0;
	double R = 0;
	double theta0 = 0;
	bool succload = 0;
};

struct FITStime
{
private:
	int startyear;
	int startmonth;
	int startday;
	int starthour;
	int startminute;
	int startsecond;

	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;

	std::string dirpath;

	std::string yearS;
	std::string monthS;
	std::string dayS;
	std::string hourS;
	std::string minuteS;
	std::string secondS;

	void timeToStringS()
	{
		yearS = to_string(year);
		if (month < 10)
		{
			monthS = "0" + to_string(month);
		}
		else
		{
			monthS = to_string(month);
		}

		if (day < 10)
		{
			dayS = "0" + to_string(day);
		}
		else
		{
			dayS = to_string(day);
		}

		if (hour < 10)
		{
			hourS = "0" + to_string(hour);
		}
		else
		{
			hourS = to_string(hour);
		}

		if (minute < 10)
		{
			minuteS = "0" + to_string(minute);
		}
		else
		{
			minuteS = to_string(minute);
		}

		if (second < 10)
		{
			secondS = "0" + to_string(second);
		}
		else
		{
			secondS = to_string(second);
		}
	}

public:
	void resetTime()
	{
		year = startyear;
		month = startmonth;
		day = startday;
		hour = starthour;
		minute = startminute;
		second = startsecond;
	}

	FITStime(std::string dirpathh, int yearr, int monthh, int dayy, int hourr, int minutee, int secondd)
	{
		dirpath = dirpathh;
		startyear = yearr;
		startmonth = monthh;
		startday = dayy;
		starthour = hourr;
		startminute = minutee;
		startsecond = secondd;
		resetTime();
		advanceTime(0);
		timeToStringS();
	}

	std::string path()
	{
		timeToStringS();
		return dirpath + yearS + "_" + monthS + "_" + dayS + "__" + hourS + "_" + minuteS + "_" + secondS + "__CONT.fits";
	}

	void advanceTime(int deltasec)
	{
		second += deltasec;
		int monthdays;
		if (month <= 7)//first seven months
		{
			if (month % 2 == 0)
				monthdays = 30;
			else
				monthdays = 31;
		}
		else//last 5 months
		{
			if (month % 2 == 0)
				monthdays = 31;
			else
				monthdays = 30;
		}
		if (month == 2)
			monthdays = 28;//february
		if ((month == 2) && (year % 4 == 0))
			monthdays = 29;//leap year fkn february

		//plus
		if (second >= 60)
		{
			minute += std::floor((double)second / 60.0);
			second %= 60;
		}
		if (minute >= 60)
		{
			hour += std::floor((double)minute / 60.0);
			minute %= 60;
		}
		if (hour >= 24)
		{
			day += std::floor((double)hour / 24.0);
			hour %= 24;
		}
		if (day >= monthdays)
		{
			month += std::floor((double)day / monthdays);
			day %= monthdays;
		}
		//minus
		if (second < 0)
		{
			minute += std::floor((double)second / 60.0);
			second = 60 + second % 60;
		}
		if (minute < 0)
		{
			hour += std::floor((double)minute / 60.0);
			minute = 60 + minute % 60;
		}
		if (hour < 0)
		{
			day += std::floor((double)hour / 24.0);
			hour = 24 + hour % 24;
		}
		if (day < 0)
		{
			month += std::floor((double)day / monthdays);
			day = monthdays + day % monthdays;
		}
	}
};

void swapbytes(char* input, unsigned length);

Mat loadfits(std::string path, fitsParams& params, fitsType type = fitsType::HMI, std::vector<std::string>* header = nullptr);

void generateFitsDownloadUrlsAndCreateFile(int delta, int step, int pics, string urlmain);

void checkFitsDownloadsAndCreateFile(int delta, int step, int pics, string urlmain, string pathMasterIn);

void loadImageDebug(Mat& activeimg, double gamaa, bool colorr, double quanBot, double quanTop);

Mat loadImage(std::string path);