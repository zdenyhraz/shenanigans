std::vector<f32> GenerateRandomVector(usize size)
{
  std::vector<f32> vec(size);
  std::ranges::for_each(vec, [](auto& x) { x = Random::Rand(0, 1); });
  return vec;
}

static void OpenCVCCSBenchmark(benchmark::State& state, std::vector<f32> input)
{
  std::vector<f32> output(input.size() / 2 + 1);
  for (auto _ : state)
  {
    cv::dft(input, output);
  }
}

static void OpenCVComplexBenchmark(benchmark::State& state, std::vector<f32> input)
{
  std::vector<std::complex<f32>> output(input.size());
  for (auto _ : state)
  {
    cv::dft(input, output, cv::DFT_COMPLEX_OUTPUT);
  }
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

    benchmark::RegisterBenchmark(fmt::format("{:>8} | OpenCV cplx", size).c_str(), OpenCVComplexBenchmark, input)->Unit(timeunit);
    benchmark::RegisterBenchmark(fmt::format("{:>8} | OpenCV ccs", size).c_str(), OpenCVCCSBenchmark, input)->Unit(timeunit);
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
