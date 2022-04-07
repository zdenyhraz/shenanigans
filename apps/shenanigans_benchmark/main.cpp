#include "Fourier/Fourier.hpp"

using Input = std::vector<cv::Mat>;

void OpenCVFFT(const Input& images)
{
  for (const auto& image : images)
    const auto fft = Fourier::fft(image.clone());
}

static void OpenCVFFTBenchmark(benchmark::State& state, const Input& images)
{
  for (auto _ : state)
  {
    OpenCVFFT(images);
  }
}

int main(int argc, char** argv)
try
{
  const auto images = LoadImages<f64>("../debug/ipcopt/train");

  benchmark::RegisterBenchmark("OpenCVFFT", OpenCVFFTBenchmark, images);

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
