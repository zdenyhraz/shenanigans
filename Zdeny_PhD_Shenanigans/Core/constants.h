#pragma once

namespace Constants {
static constexpr double Pi = 3.1415926535897932384626433;
static constexpr double TwoPi = Pi * 2;
static constexpr double HalfPi = Pi / 2;
static constexpr double QuartPi = Pi / 4;
static constexpr double E = 2.7182818284590452353602874;
static constexpr double Max = std::numeric_limits<double>::max();
static constexpr double Min = std::numeric_limits<double>::lowest();
static constexpr double Eps = std::numeric_limits<double>::epsilon();

static const std::vector<double> emptyvect = std::vector<double> {};
static const std::vector<std::vector<double>> emptyvect2 = std::vector<std::vector<double>> {};
static const std::string emptystring = "";
static const std::vector<std::string> emptyvectstring = std::vector<std::string> {};
}