#include "OnnxModel.hpp"

OnnxModel::OnnxModel(const std::filesystem::path& modelPath)
{
  Load(modelPath);
}

void OnnxModel::Load(const std::filesystem::path& modelPath)
try
{
  const auto modelPathEx = GetExistingPath(modelPath.string());
  if (not std::filesystem::is_regular_file(modelPathEx))
    throw EXCEPTION("Could not find model '{}'", modelPathEx.string());

  env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "Umbellula detector");
  options = Ort::SessionOptions();
  options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
  LoadProviders(options);
  model = Ort::Session(env, modelPathEx.c_str(), options);
  memoryInfo = Ort::MemoryInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault));
  loaded = true;
  LOG_DEBUG("Loaded model {}", modelPathEx.string());
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}

void OnnxModel::Unload()
{
  memoryInfo = Ort::MemoryInfo(nullptr);
  model = Ort::Session(nullptr);
  options = Ort::SessionOptions(nullptr);
  env = Ort::Env(nullptr);
  loaded = false;
}

void OnnxModel::LoadProviders(Ort::SessionOptions& options)
{
  auto providers = Ort::GetAvailableProviders();
  LOG_DEBUG("Available providers:");
  for (const auto& provider : providers)
    LOG_DEBUG("  - {}", provider);

  static constexpr int deviceId = 0;
  if (false and std::ranges::contains(providers, "TensorrtExecutionProvider"))
    try
    {
      OrtTensorRTProviderOptions tensorrtOptions;
      tensorrtOptions.device_id = deviceId;
      options.AppendExecutionProvider_TensorRT(tensorrtOptions);
      LOG_DEBUG("ONNX TensorRT provider loaded");
    }
    catch (const std::exception& e)
    {
      LOG_WARNING("ONNX TensorRT provider failed to load: {}", e.what());
    }
  if (std::ranges::contains(providers, "CUDAExecutionProvider"))
    try
    {
      OrtCUDAProviderOptions cudaOptions;
      cudaOptions.device_id = deviceId;
      options.AppendExecutionProvider_CUDA(cudaOptions);
      LOG_DEBUG("ONNX CUDA provider loaded");
    }
    catch (const std::exception& e)
    {
      LOG_WARNING("ONNX CUDA provider failed to load: {}", e.what());
    }
  if (std::ranges::contains(providers, "CPUExecutionProvider"))
  {
    LOG_DEBUG("ONNX CPU provider loaded");
  }
}

cv::Mat OnnxModel::Preprocess(const cv::Mat& image)
{
  // HWC BGR 8U -> CHW RGB 32F
  int height = image.rows;
  int width = image.cols;
  cv::Mat chw(3 * height * width, 1, CV_8UC1);
  uchar* chwData = chw.ptr<uchar>(0);
  cv::Mat chwChannels[3] = {
      cv::Mat(height, width, CV_8UC1, chwData),                     // R plane
      cv::Mat(height, width, CV_8UC1, chwData + height * width),    // G plane
      cv::Mat(height, width, CV_8UC1, chwData + 2 * height * width) // B plane
  };

  int channelMap[] = {2, 0, 1, 1, 0, 2};
  cv::mixChannels(&image, 1, chwChannels, 3, channelMap, 3);
  chw.convertTo(chw, CV_32FC1, 1.0 / 255.0);
  return chw;
}

std::vector<Ort::Value> OnnxModel::Run(const cv::Mat image)
{
  std::vector<int64_t> inputShape = {1, 3, image.rows, image.cols};
  cv::Mat imageTensor = Preprocess(image);
  Ort::Value inputTensor =
      Ort::Value::CreateTensor<float>(memoryInfo, imageTensor.ptr<float>(0), imageTensor.total() * imageTensor.channels(), inputShape.data(), inputShape.size());
  return model.Run(Ort::RunOptions{nullptr}, inputNames.data(), &inputTensor, 1, outputNames.data(), 3);
}
