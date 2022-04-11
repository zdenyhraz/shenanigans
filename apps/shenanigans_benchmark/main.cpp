#include "Fourier/Fourier.hpp"
#include <fftw3.h>

// state.PauseTiming();
// state.ResumeTiming();

using Input = std::vector<cv::Mat>;

static void OpenCVFFTComplexBenchmark(benchmark::State& state, const Input& images)
{
  cv::Mat fft;
  for (auto _ : state)
  {
    for (const auto& image : images)
      cv::dft(image, fft, cv::DFT_COMPLEX_OUTPUT);
  }
}

static void OpenCVFFTCSSBenchmark(benchmark::State& state, const Input& images)
{
  cv::Mat fft;
  for (auto _ : state)
  {
    for (const auto& image : images)
      cv::dft(image, fft);
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
    auto images = LoadImages<f64>("../debug/ipcopt/train");
    ResizeImages(images, {size, 1});
    benchmark::RegisterBenchmark(fmt::format("{:>8} | OpenCV Complex", size).c_str(), OpenCVFFTComplexBenchmark, images)->Unit(timeunit);
    benchmark::RegisterBenchmark(fmt::format("{:>8} | OpenCV CCS", size).c_str(), OpenCVFFTCSSBenchmark, images)->Unit(timeunit);
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
