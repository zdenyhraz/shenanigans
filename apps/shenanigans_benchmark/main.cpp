#include "Fourier/Fourier.hpp"
#include <fftw3.h>

// state.PauseTiming();
// state.ResumeTiming();

using Precision = f64;

std::vector<Precision> GenerateRandomVector(usize size)
{
  std::vector<Precision> vec(size);
  for (auto& x : vec)
    x = Random::Rand(0, 1);
  return vec;
}

static void OpenCVBenchmark(benchmark::State& state, std::vector<Precision> input, std::vector<Precision> output)
{
  cv::Mat inputWrapper(1, input.size(), GetMatType<Precision>(1), input.data());
  cv::Mat outputWrapper(1, input.size(), GetMatType<Precision>(2), output.data());

  for (auto _ : state)
  {
    cv::dft(inputWrapper, outputWrapper, cv::DFT_COMPLEX_OUTPUT);
  }
}

static void FFTWBenchmark(benchmark::State& state, std::vector<Precision> input, std::vector<Precision> output)
{
  // from https://www.fftw.org/fftw3_doc/One_002dDimensional-DFTs-of-Real-Data.html:
  // In many practical applications, the input data in[i] are purely real numbers, in which case the DFT output satisfies the “Hermitian” redundancy: out[i] is the conjugate of out[n-i]. It is
  // possible to take advantage of these circumstances in order to achieve roughly a factor of two improvement in both speed and memory usage. In exchange for these speed and space advantages, the
  // user sacrifices some of the simplicity of FFTW’s complex transforms. First of all, the input and output arrays are of different sizes and types: the input is n real numbers, while the output is
  // n/2+1 complex numbers (the non-redundant outputs); this also requires slight “padding” of the input array for in-place transforms. Second, the inverse transform (complex to real) has the
  // side-effect of overwriting its input array, by default. Neither of these inconveniences should pose a serious problem for users, but it is important to be aware of them.
  fftw_plan plan = fftw_plan_dft_r2c_1d(input.size(), input.data(), reinterpret_cast<fftw_complex*>(output.data()), FFTW_ESTIMATE);
  for (auto _ : state)
  {
    fftw_execute(plan);
  }
  fftw_destroy_plan(plan);
  fftw_cleanup();
}

int main(int argc, char** argv)
try
{
  static constexpr auto timeunit = benchmark::kMillisecond;
  static constexpr auto expmin = 19;
  static constexpr auto expmax = 24;
  static const auto exponents = Linspace<i32>(expmin, expmax, expmax - expmin + 1);

  for (const auto exponent : exponents)
  {
    const auto size = 1 << exponent;
    const auto input = GenerateRandomVector(size);      // real
    const auto output = GenerateRandomVector(size * 2); // complex

    benchmark::RegisterBenchmark(fmt::format("{:>8} | OpenCV", size).c_str(), OpenCVBenchmark, input, output)->Unit(timeunit);
    benchmark::RegisterBenchmark(fmt::format("{:>8} | FFTW", size).c_str(), FFTWBenchmark, input, output)->Unit(timeunit);
  }

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();

  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
  return EXIT_FAILURE;
}
catch (...)
{
  LOG_UNKNOWN_EXCEPTION;
  return EXIT_FAILURE;
}
