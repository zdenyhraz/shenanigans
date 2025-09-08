#include "OnnxModel.hpp"

OnnxModel::OnnxModel(const std::filesystem::path& modelPath, const char* _name, const std::vector<const char*>& _inputNames, const std::vector<const char*>& _outputNames) :
  name(_name), inputNames(_inputNames), outputNames(_outputNames)
{
  logName = fmt::format("[{}/{}] ONNX", name, fmt::ptr(this));
  if (not modelPath.empty())
    Load(modelPath);
}

void OnnxModel::VerifyInputsOutputs()
{
  size_t inputCount = session.GetInputCount();
  if (inputNames.size() != inputCount)
    throw EXCEPTION("{} model input count mismatch: Expected {} inputs, but model has {}", logName, inputNames.size(), inputCount);

  size_t outputCount = session.GetOutputCount();
  if (outputNames.size() != outputCount)
    throw EXCEPTION("{} model output count mismatch: Expected {} outputs, but model has {}", logName, outputNames.size(), outputCount);

  Ort::AllocatorWithDefaultOptions allocator;

  for (size_t i = 0; i < inputCount; ++i)
    if (const auto modelInputName = session.GetInputNameAllocated(i, allocator); std::strcmp(inputNames[i], modelInputName.get()) != 0)
      throw EXCEPTION("{} model input name mismatch at index {}: Expected '{}', but got '{}'", logName, i, inputNames[i], modelInputName.get());

  for (size_t i = 0; i < outputCount; ++i)
    if (const auto modelOutputName = session.GetOutputNameAllocated(i, allocator); std::strcmp(outputNames[i], modelOutputName.get()) != 0)
      throw EXCEPTION("{} model output name mismatch at index {}: Expected '{}', but got '{}'", logName, i, outputNames[i], modelOutputName.get());
}

void OnnxModel::Load(const std::filesystem::path& modelPath)
{
  LOG_DEBUG("{} loading model {}", logName, modelPath);
  if (not std::filesystem::is_regular_file(modelPath))
    throw EXCEPTION("Could not find model '{}'", modelPath.string());

  LOG_DEBUG("Initializing ONNX Runtime");
  env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, name, OnnxLogFunction, nullptr);
  options = Ort::SessionOptions();
  options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
  LoadProviders();
  session = Ort::Session(env, modelPath.c_str(), options);
  memoryInfo = Ort::MemoryInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, usesGPU ? OrtMemTypeCPUInput : OrtMemTypeDefault));
  loaded = true;
  VerifyInputsOutputs();
  LOG_DEBUG("{} loaded model {} | GPU: {}", logName, modelPath, usesGPU);
}

void OnnxModel::Unload()
{
  memoryInfo = Ort::MemoryInfo(nullptr);
  session = Ort::Session(nullptr);
  options = Ort::SessionOptions(nullptr);
  env = Ort::Env(nullptr);
  usesGPU = false;
  loaded = false;
  LOG_DEBUG("{} model unloaded", logName);
}

void OnnxModel::LoadProviders()
{
  LOG_DEBUG("Loading ONNX providers");
  const auto providers = Ort::GetAvailableProviders();
  LOG_DEBUG("{} available providers: {}", logName, providers);

  if (useCUDA and std::ranges::contains(providers, "CUDAExecutionProvider"))
    try
    {
      OrtCUDAProviderOptions cudaOptions;
      options.AppendExecutionProvider_CUDA(cudaOptions);
      usesGPU = true;
      LOG_DEBUG("{} CUDA provider loaded", logName);
    }
    catch (const std::exception& e)
    {
      LOG_WARNING("{} CUDA provider failed to load: {}", logName, e.what());
    }
  if (useTensorRT and std::ranges::contains(providers, "TensorrtExecutionProvider"))
    try
    {
      OrtTensorRTProviderOptions tensorrtOptions;
      options.AppendExecutionProvider_TensorRT(tensorrtOptions);
      usesGPU = true;
      LOG_DEBUG("{} TensorRT provider loaded", logName);
    }
    catch (const std::exception& e)
    {
      LOG_WARNING("{} TensorRT provider failed to load: {}", logName, e.what());
    }
  if (std::ranges::contains(providers, "CPUExecutionProvider"))
  {
    LOG_DEBUG("{} CPU provider loaded", logName);
  }
}

void OnnxModel::OnnxLogFunction(void* param, OrtLoggingLevel severity, const char* category, const char* logid, const char* code_location, const char* message)
{
  switch (severity)
  {
  case ORT_LOGGING_LEVEL_VERBOSE:
    return LOG_DEBUG("ONNX {}", message);
  case ORT_LOGGING_LEVEL_INFO:
    return LOG_INFO("ONNX {}", message);
  case ORT_LOGGING_LEVEL_WARNING:
    return LOG_WARNING("ONNX {}", message);
  case ORT_LOGGING_LEVEL_ERROR:
    return LOG_ERROR("ONNX {}", message);
  case ORT_LOGGING_LEVEL_FATAL:
    return LOG_ERROR("ONNX {}", message);
  }
};

void OnnxModel::Preprocess(const cv::Mat& image)
{
  cv::dnn::blobFromImage(image, imageTensor, 1.0 / 255.0);
  std::vector<int64_t> inputShape = {1, 3, image.rows, image.cols};
  inputTensor = Ort::Value::CreateTensor<float>(memoryInfo, imageTensor.ptr<float>(0), imageTensor.total() * imageTensor.channels(), inputShape.data(), inputShape.size());
}

std::vector<Ort::Value> OnnxModel::Run(const cv::Mat& image)
{
  Preprocess(image);
  return session.Run(Ort::RunOptions{nullptr}, inputNames.data(), &inputTensor, inputNames.size(), outputNames.data(), outputNames.size());
}
