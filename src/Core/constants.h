#pragma once

namespace Constants
{
static constexpr f64 Pi = 3.1415926535897932384626433;
static constexpr f64 TwoPi = Pi * 2;
static constexpr f64 HalfPi = Pi / 2;
static constexpr f64 QuartPi = Pi / 4;
static constexpr f64 E = 2.7182818284590452353602874;
static constexpr f64 Rad = 360. / TwoPi;
static constexpr f64 SecondsInDay = 24. * 60. * 60.;
static constexpr f64 RadPerSecToDegPerDay = Rad * SecondsInDay;
static constexpr f64 Inf = std::numeric_limits<f64>::max();
static constexpr f64 IntInf = std::numeric_limits<i32>::max();

static const std::vector<f64> emptyvect = std::vector<f64>{};
static const std::vector<std::vector<f64>> emptyvect2 = std::vector<std::vector<f64>>{};
static const std::string emptystring = "";
static const std::vector<std::string> emptyvectstring = std::vector<std::string>{};
}
