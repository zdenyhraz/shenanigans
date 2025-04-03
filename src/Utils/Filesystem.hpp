#pragma once

inline std::filesystem::path GetProjectPath(std::string_view relpath = "")
{
  return std::filesystem::path(PROJECT_DIRECTORY) / relpath;
}

inline size_t GetFileCount(const std::filesystem::path& dirpath)
{
  if (not std::filesystem::is_directory(dirpath))
    throw std::invalid_argument(fmt::format("{} is not a valid directory", dirpath.string()));
  return std::ranges::count_if(std::filesystem::directory_iterator(dirpath), [](const auto& entry) { return entry.is_regular_file(); });
}

inline size_t GetRecursiveFileCount(const std::filesystem::path& dirpath)
{
  if (not std::filesystem::is_directory(dirpath))
    throw std::invalid_argument(fmt::format("{} is not a valid directory", dirpath.string()));
  return std::ranges::count_if(std::filesystem::recursive_directory_iterator(dirpath), [](const auto& entry) { return entry.is_regular_file(); });
}

inline size_t GetDirectoryCount(const std::filesystem::path& dirpath)
{
  if (not std::filesystem::is_directory(dirpath))
    throw std::invalid_argument(fmt::format("{} is not a valid directory", dirpath.string()));
  return std::ranges::count_if(std::filesystem::directory_iterator(dirpath), [](const auto& entry) { return entry.is_directory(); });
}

class FilePathGenerator
{
  std::string mDirPath;
  std::filesystem::recursive_directory_iterator mIterator;
  std::vector<std::string> allowedExtensions;

public:
  FilePathGenerator() = default;

  FilePathGenerator(const std::string& dirPath) : mDirPath(dirPath), mIterator(dirPath)
  {
    if (not std::filesystem::is_directory(mDirPath))
      throw std::invalid_argument(fmt::format("{} is not a valid directory", mDirPath));
  }

  void Reset() { mIterator = std::filesystem::recursive_directory_iterator(mDirPath); }

  void SetDirectory(const std::string& dirPath)
  {
    if (dirPath != mDirPath)
    {
      if (not std::filesystem::is_directory(dirPath))
        throw std::invalid_argument(fmt::format("{} is not a valid directory", dirPath));

      mDirPath = dirPath;
      Reset();
    }
  }

  bool Valid() { return mIterator != std::filesystem::end(mIterator); }

  std::string GetDirectory() { return mDirPath; }

  void AddExtensionFilter(const std::string& extension) { allowedExtensions.push_back(extension); }

  void ResetExtensionFilter() { allowedExtensions.clear(); }

  std::filesystem::path GetNextFilePath()
  {
    while (Valid())
    {
      if (mIterator->is_regular_file())
      {
        const auto path = mIterator->path();
        if (allowedExtensions.empty() or std::ranges::find(allowedExtensions, path.extension().string()) != allowedExtensions.end())
        {
          ++mIterator;
          return path;
        }
      }
      ++mIterator;
    }
    return {};
  }
};
