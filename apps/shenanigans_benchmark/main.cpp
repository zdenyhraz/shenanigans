#include "Fourier/Fourier.hpp"

using Input = std::vector<cv::Mat>;

void OpenCVFFTComplex(const Input& images)
{
  cv::Mat fft;
  for (const auto& image : images)
    cv::dft(image, fft, cv::DFT_COMPLEX_OUTPUT);
}

void OpenCVFFTCSS(const Input& images)
{
  cv::Mat fft;
  for (const auto& image : images)
    cv::dft(image, fft);
}

static void OpenCVFFTComplexBenchmark(benchmark::State& state, const Input& images)
{
  for (auto _ : state)
    OpenCVFFTComplex(images);
}

static void OpenCVFFTCSSBenchmark(benchmark::State& state, const Input& images)
{
  // state.PauseTiming();
  // state.ResumeTiming();
  for (auto _ : state)
    OpenCVFFTCSS(images);
}

int main(int argc, char** argv)
try
{
  static constexpr usize iterations = 100;
  static constexpr benchmark::TimeUnit timeunit = benchmark::kMicrosecond;
  const auto images = LoadImages<f64>("../debug/ipcopt/train");

  benchmark::RegisterBenchmark("OpenCV Complex", OpenCVFFTComplexBenchmark, images)->Iterations(iterations)->Unit(timeunit);
  benchmark::RegisterBenchmark("OpenCV CSS", OpenCVFFTCSSBenchmark, images)->Iterations(iterations)->Unit(timeunit);

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
  LOG_ERROR("Error: Unknown error");
  return EXIT_FAILURE;
}
