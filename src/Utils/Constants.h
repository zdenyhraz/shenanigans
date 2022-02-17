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
static constexpr f64 Infi32 = std::numeric_limits<i32>::max();
}
