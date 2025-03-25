#include "OnnxModel.hpp"

OnnxModel::OnnxModel(const char* name, const std::filesystem::path& modelPath, const std::vector<const char*>& _inputNames, const std::vector<const char*>& _outputNames)
{
  SetName(name);
  if (not modelPath.empty())
    Load(modelPath);
  SetInputNames(_inputNames);
  SetOutputNames(_outputNames);
}

void OnnxModel::Load(const std::filesystem::path& modelPath)
try
{
  const auto modelPathEx = GetExistingPath(modelPath.string());
  if (not std::filesystem::is_regular_file(modelPathEx))
    throw EXCEPTION("Could not find model '{}'", modelPathEx.string());

  env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, name);
  options = Ort::SessionOptions();
  options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
  LoadProviders();
  session = Ort::Session(env, modelPathEx.c_str(), options);
  memoryInfo = Ort::MemoryInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, usesGPU ? OrtMemTypeCPUInput : OrtMemTypeDefault));
  loaded = true;
  LOG_DEBUG("Loaded model {} | GPU: {}", modelPathEx.string(), usesGPU);
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}

void OnnxModel::Unload()
try
{
  memoryInfo = Ort::MemoryInfo(nullptr);
  session = Ort::Session(nullptr);
  options = Ort::SessionOptions(nullptr);
  env = Ort::Env(nullptr);
  usesGPU = false;
  loaded = false;
  LOG_DEBUG("Model unloaded");
}
catch (const std::exception& e)
{
  LOG_EXCEPTION(e);
}

void OnnxModel::LoadProviders()
{
  const auto providers = Ort::GetAvailableProviders();
  LOG_DEBUG("Available providers: {}", providers);

  static constexpr int deviceId = 0;
  if (false and std::ranges::contains(providers, "TensorrtExecutionProvider"))
    try
    {
      OrtTensorRTProviderOptions tensorrtOptions;
      tensorrtOptions.device_id = deviceId;
      options.AppendExecutionProvider_TensorRT(tensorrtOptions);
      usesGPU = true;
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
      usesGPU = true;
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
      cv::Mat(height, width, CV_8UC1, chwData),                     // R
      cv::Mat(height, width, CV_8UC1, chwData + height * width),    // G
      cv::Mat(height, width, CV_8UC1, chwData + 2 * height * width) // B
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
  return session.Run(Ort::RunOptions{nullptr}, inputNames.data(), &inputTensor, 1, outputNames.data(), 3);
}
