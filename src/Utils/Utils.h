#pragma once

inline f64 rand01()
{
  return (f64)rand() / RAND_MAX;
}

inline f64 rand11()
{
  return 2.0 * rand01() - 1.0;
}

inline f64 randr(f64 min_, f64 max_)
{
  return min_ + rand01() * (max_ - min_);
}

inline f64 clamp(f64 x, f64 clampMin, f64 clampMax)
{
  return std::min(std::max(x, clampMin), clampMax);
}

inline f64 clampSmooth(f64 x_new, f64 x_prev, f64 clampMin, f64 clampMax)
{
  return x_new < clampMin ? (x_prev + clampMin) / 2 : x_new > clampMax ? (x_prev + clampMax) / 2 : x_new;
}

template <typename T = f64>
inline auto zerovect(i32 N, T value = 0.)
{
  return std::vector<T>(N, value);
}

template <typename T = f64>
inline auto zerovect2(i32 N, i32 M, T value = 0.)
{
  return std::vector<std::vector<T>>(N, zerovect(M, value));
}

template <typename T>
inline constexpr T sqr(T x)
{
  return x * x;
}

template <typename T>
inline f64 mean(const std::vector<T>& vec)
{
  f64 mean = 0;
  for (auto& x : vec)
    mean += x;
  return mean / vec.size();
}

template <typename T>
inline f64 mean(const std::vector<std::vector<T>>& vec)
{
  f64 mean = 0;
  for (auto& row : vec)
    for (auto& x : row)
      mean += x;
  return mean / vec.size() / vec[0].size();
}

template <typename T>
inline f64 stdev(const std::vector<T>& vec)
{
  f64 m = mean(vec);
  f64 stdev = 0;
  for (auto& x : vec)
    stdev += sqr(x - m);
  stdev /= vec.size();
  return sqrt(stdev);
}

template <typename T>
inline f64 stdevs(const std::vector<T>& vec)
{
  f64 m = mean(vec);
  f64 stdev = 0;
  for (auto& x : vec)
    stdev += sqr(x - m);
  stdev /= (vec.size() - 1);
  return sqrt(stdev);
}

template <typename T>
inline std::vector<T> operator+(const std::vector<T>& vec1, const std::vector<T>& vec2)
{
  std::vector<T> result = vec1;
  for (usize i = 0; i < vec1.size(); i++)
    result[i] = vec1[i] + vec2[i];
  return result;
}

template <typename T>
inline std::vector<T> operator-(const std::vector<T>& vec1, const std::vector<T>& vec2)
{
  std::vector<T> result = vec1;
  for (usize i = 0; i < vec1.size(); i++)
    result[i] = vec1[i] - vec2[i];
  return result;
}

template <typename T>
inline std::vector<T>& operator+=(std::vector<T>& vec1, const std::vector<T>& vec2)
{
  for (usize i = 0; i < vec1.size(); i++)
    vec1[i] += vec2[i];
  return vec1;
}

template <typename T>
inline std::vector<T>& operator-=(std::vector<T>& vec1, const std::vector<T>& vec2)
{
  for (usize i = 0; i < vec1.size(); i++)
    vec1[i] -= vec2[i];
  return vec1;
}

template <typename T>
inline std::vector<T> operator*(f64 val, const std::vector<T>& vec)
{
  std::vector<T> result = vec;
  for (usize i = 0; i < vec.size(); i++)
    result[i] = val * vec[i];
  return result;
}

template <typename T>
inline std::vector<T> abs(const std::vector<T>& vec)
{
  auto result = vec;
  for (usize i = 0; i < vec.size(); i++)
    result[i] = abs(vec[i]);
  return result;
}

template <typename T>
inline f64 median(const std::vector<T>& vec)
{
  auto result = vec;
  std::sort(result.begin(), result.end());
  return result[result.size() / 2];
}

inline bool vectorLess(const std::vector<f64>& left, const std::vector<f64>& right)
{
  f64 L = 0, R = 0;
  for (usize i = 0; i < left.size(); i++)
  {
    L += abs(left[i]);
    R += abs(right[i]);
  }
  return L < R;
}

template <typename T>
inline T vectorMax(const std::vector<T>& input)
{
  T vectmax = input[0];
  for (usize i = 0; i < input.size(); i++)
  {
    if (input[i] > vectmax)
      vectmax = input[i];
  }
  return vectmax;
}

template <typename T>
inline T vectorMin(const std::vector<T>& input)
{
  T vectmin = input[0];
  for (usize i = 0; i < input.size(); i++)
  {
    if (input[i] < vectmin)
      vectmin = input[i];
  }
  return vectmin;
}

template <typename T>
inline T arrayMax(T* input, u32 size)
{
  T arrmax = input[0];
  for (usize i = 0; i < size; i++)
  {
    if (input[i] > arrmax)
      arrmax = input[i];
  }
  return arrmax;
}

template <typename T>
inline T arrayMin(T* input, u32 size)
{
  T arrmin = input[0];
  for (usize i = 0; i < size; i++)
  {
    if (input[i] < arrmin)
      arrmin = input[i];
  }
  return arrmin;
}

inline std::string currentDateTime()
{
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  strftime(buf, sizeof(buf), "%Y-%b%d-%H.%M.%S", &tstruct);
  return buf;
}

inline std::string currentTime()
{
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  strftime(buf, sizeof(buf), "%H:%M:%S", &tstruct);
  return buf;
}

inline f64 gaussian1D(f64 x, f64 amp, f64 mu, f64 sigma)
{
  return amp * exp(-0.5 * pow((x - mu) / sigma, 2));
}

constexpr i32 factorial(i32 n)
{
  return (n == 1 or n == 0) ? 1 : factorial(n - 1) * n;
}

inline void linreg(i32 n, const std::vector<f64>& x, const std::vector<f64>& y, f64& k, f64& q)
{
  f64 sumx = 0.;
  f64 sumx2 = 0.;
  f64 sumxy = 0.;
  f64 sumy = 0.;

  for (usize i = 0; i < n; i++)
  {
    sumx += x[i];
    sumx2 += sqr(x[i]);
    sumxy += x[i] * y[i];
    sumy += y[i];
  }

  f64 denom = (n * sumx2 - sqr(sumx));

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

inline f64 linregPosition(i32 n, const std::vector<f64>& x, const std::vector<f64>& y, f64 x_)
{
  f64 k, q;
  linreg(n, x, y, k, q);
  return k * x_ + q;
}

inline std::vector<f64> Iota(i32 first, i32 size)
{
  std::vector<f64> vec(size);
  std::iota(vec.begin(), vec.end(), first);
  return vec;
}

inline std::string operator+(const std::string& str, const i32 val)
{
  return str + std::to_string(val);
}

inline std::string operator+(const i32 val, const std::string& str)
{
  return std::to_string(val) + str;
}

inline void filterMedian(std::vector<f64>& vec, i32 size)
{
  std::vector<f64> vecMedian = vec;
  std::vector<f64> med;
  med.reserve(size);

  for (usize i = 0; i < vec.size(); i++)
  {
    med.clear();
    for (i32 m = 0; m < size; m++)
    {
      i32 idx = i - size / 2 + m;

      if (idx < 0)
        continue;
      if (idx == vec.size())
        break;

      med.emplace_back(vec[idx]);
    }

    vecMedian[i] = median(med);
  }
  vec = vecMedian;
}

inline void filterMovavg(std::vector<f64>& vec, i32 size)
{
  std::vector<f64> vecMovavg = vec;
  f64 movavg;
  i32 movavgcnt;

  for (usize i = 0; i < vec.size(); i++)
  {
    movavg = 0;
    movavgcnt = 0;
    for (i32 m = 0; m < size; m++)
    {
      i32 idx = i - size / 2 + m;

      if (idx < 0)
        continue;
      if (idx == vec.size())
        break;

      movavg += vec[idx];
      movavgcnt += 1;
    }
    movavg /= (f64)movavgcnt;
    vecMovavg[i] = movavg;
  }
  vec = vecMovavg;
}

inline f64 toDegrees(f64 rad)
{
  return rad * Constants::Rad;
}

inline f64 toRadians(f64 deg)
{
  return deg / Constants::Rad;
}

inline std::vector<f64> toDegrees(const std::vector<f64>& vecrad)
{
  auto vecdeg = vecrad;
  for (usize i = 0; i < vecrad.size(); i++)
  {
    vecdeg[i] = toDegrees(vecrad[i]);
  }
  return vecdeg;
}

inline std::vector<f64> toRadians(const std::vector<f64>& vecdeg)
{
  auto vecrad = vecdeg;
  for (usize i = 0; i < vecdeg.size(); i++)
  {
    vecrad[i] = toRadians(vecdeg[i]);
  }
  return vecrad;
}

inline std::vector<f64> removeQuantileOutliers(const std::vector<f64>& vec, f64 quanB, f64 quanT)
{
  auto out = vec;
  std::sort(out.begin(), out.end());
  return std::vector<f64>(out.begin() + (i32)(quanB * (out.size() - 1)), out.begin() + (i32)(quanT * (out.size() - 1)));
}

inline f64 getQuantile(const std::vector<f64>& vec, f64 quan)
{
  std::vector<f64> out = vec;
  std::sort(out.begin(), out.end());
  return out[(i32)(quan * (out.size() - 1))];
}

inline f64 getQuantile(const std::vector<std::vector<f64>>& vec, f64 quan)
{
  std::vector<f64> out;
  out.reserve(vec.size() * vec[0].size());
  for (usize i = 0; i < vec.size(); i++)
    for (f64 x : vec[i])
      out.push_back(x);
  std::sort(out.begin(), out.end());
  return out[(i32)(quan * (out.size() - 1))];
}

inline std::vector<f64> getStandardErrorsOfTheMeanHorizontal(const std::vector<std::vector<f64>>& vec)
{
  std::vector<f64> errors(vec.size());

  for (usize i = 0; i < vec.size(); i++)
  {
    f64 m = mean(vec[i]);
    f64 s = stdev(vec[i]);
    f64 n = vec[i].size();
    f64 e = s / sqrt(n);
    errors[i] = e;
  }
  return errors;
}

inline std::vector<f64> getStandardErrorsOfTheMeanVertical(const std::vector<std::vector<f64>>& vec)
{
  std::vector<f64> errors(vec[0].size());

  for (usize i = 0; i < vec[0].size(); i++)
  {
    std::vector<f64> v(vec.size());
    for (i32 j = 0; j < vec.size(); j++)
      v[j] = vec[j][i];

    f64 m = mean(v);
    f64 s = stdev(v);
    f64 n = v.size();
    f64 e = s / sqrt(n);
    errors[i] = e;
  }
  return errors;
}

inline std::vector<f64> getStandardDeviationsVertical(const std::vector<std::vector<f64>>& vec)
{
  std::vector<f64> stdevs(vec[0].size());

  for (usize i = 0; i < vec[0].size(); i++)
  {
    std::vector<f64> v(vec.size());
    for (i32 j = 0; j < vec.size(); j++)
      v[j] = vec[j][i];

    stdevs[i] = stdev(v);
  }
  return stdevs;
}

inline cv::Mat LeastSquares(const cv::Mat& Y, const cv::Mat& X)
{
  if (Y.rows != X.rows)
    throw std::runtime_error("Data count mismatch");

  return (X.t() * X).inv() * X.t() * Y;
}