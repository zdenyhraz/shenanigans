#include "Fourier/Fourier.hpp"
#include <fftw3.h>

// state.PauseTiming();
// state.ResumeTiming();

std::vector<f32> GenerateRandomVector(usize size)
{
  std::vector<f32> vec(size);
  for (auto& x : vec)
    x = Random::Rand(0, 1);
  return vec;
}

static void OpenCVFFTBenchmark(benchmark::State& state, std::vector<f32> input, std::vector<f32> output)
{
  cv::Mat inputWrapper(1, input.size(), GetMatType<f32>(), input.data());
  cv::Mat outputWrapper(1, input.size(), GetMatType<f32>(2), output.data());

  for (auto _ : state)
  {
    cv::dft(inputWrapper, outputWrapper, cv::DFT_COMPLEX_OUTPUT);
  }
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

    benchmark::RegisterBenchmark(fmt::format("{:>8} | OpenCV", size).c_str(), OpenCVFFTBenchmark, input, output)->Unit(timeunit);
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
