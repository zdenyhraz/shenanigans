#include <fftw3.h>
#include "fftw_vector.hpp"

std::vector<f32> GenerateRandomVector(usize size)
{
  std::vector<f32> vec(size);
  std::ranges::for_each(vec, [](auto& x) { x = Random::Rand(0, 1); });
  return vec;
}

static void OpenCVBenchmark(benchmark::State& state, std::vector<f32> input)
{
  std::vector<f32> output(input.size() / 2 + 1);
  for (auto _ : state)
  {
    cv::dft(input, output, cv::DFT_ROWS);
  }
}

static void FFTWBenchmark(benchmark::State& state, std::vector<f32> input)
{
  // from https://www.fftw.org/fftw3_doc/One_002dDimensional-DFTs-of-Real-Data.html
  // In many practical applications, the input data in[i] are purely real numbers, in which case the DFT output satisfies the “Hermitian” redundancy:
  // out[i] is the conjugate of out[n-i]. It is possible to take advantage of these circumstances in order to achieve roughly a factor of two
  // improvement in both speed and memory usage. In exchange for these speed and space advantages, the user sacrifices some of the simplicity of
  // FFTW’s complex transforms. First of all, the input and output arrays are of different sizes and types: the input is n real numbers, while the
  // output is n/2+1 complex numbers (the non-redundant outputs); this also requires slight “padding” of the input array for in-place transforms.
  // Second, the inverse transform (complex to real) has the side-effect of overwriting its input array, by default. Neither of these inconveniences
  // should pose a serious problem for users, but it is important to be aware of them.

  fftw::vector<f32> inputAligned(input.size());
  fftw::vector<fftwf_complex> outputAligned(input.size() / 2 + 1);
  std::memcpy(inputAligned.data(), input.data(), input.size());
  fftwf_plan plan = fftwf_plan_dft_r2c_1d(input.size(), inputAligned.data(), outputAligned.data(), FFTW_ESTIMATE);
  for (auto _ : state)
  {
    fftwf_execute(plan);
  }
  fftwf_destroy_plan(plan);
  fftwf_cleanup();
}

// --benchmark_out_format={json|console|csv}
// --benchmark_out=<filename>
int main(int argc, char** argv)
try
{
  static constexpr auto expmin = 8;  // 8=512
  static constexpr auto expmax = 24; // 24=16M
  static const auto exponents = Linspace<i32>(expmin, expmax, expmax - expmin + 1);

  for (const auto exponent : exponents)
  {
    const auto timeunit = exponent > 18 ? benchmark::kMillisecond : benchmark::kMicrosecond;
    const auto size = 1 << exponent;
    const auto input = GenerateRandomVector(size); // real

    benchmark::RegisterBenchmark(fmt::format("{:>8} | OpenCV-IPP ccs", size).c_str(), OpenCVBenchmark, input)->Unit(timeunit);
    benchmark::RegisterBenchmark(fmt::format("{:>8} | FFTW r2c", size).c_str(), FFTWBenchmark, input)->Unit(timeunit);
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
