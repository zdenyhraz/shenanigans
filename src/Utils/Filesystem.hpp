#pragma once

inline std::filesystem::path GetProjectDirectoryPath()
{
  return std::filesystem::path(PROJECT_DIRECTORY);
}

inline std::filesystem::path GetProjectDirectoryPath(std::string_view relpath)
{
  return std::filesystem::path(PROJECT_DIRECTORY) / relpath;
}

inline std::filesystem::path GetExistingPath(std::string_view relpath)
{
  const auto projectDirectoryPath = GetProjectDirectoryPath(relpath);
  if (std::filesystem::exists(projectDirectoryPath))
    return projectDirectoryPath;
  return relpath;
}

inline usize GetFileCount(const std::filesystem::path& dirpath)
{
  if (not std::filesystem::is_directory(dirpath))
    throw std::invalid_argument(fmt::format("{} is not a valid directory", dirpath.string()));
  return std::ranges::count_if(std::filesystem::directory_iterator(dirpath), [](const auto& entry) { return entry.is_regular_file(); });
}

inline usize GetDirectoryCount(const std::filesystem::path& dirpath)
{
  if (not std::filesystem::is_directory(dirpath))
    throw std::invalid_argument(fmt::format("{} is not a valid directory", dirpath.string()));
  return std::ranges::count_if(std::filesystem::directory_iterator(dirpath), [](const auto& entry) { return entry.is_directory(); });
}

class FilePathGenerator
{
  std::string mDirPath;
  std::filesystem::directory_iterator mIterator;

public:
  FilePathGenerator() = default;

  FilePathGenerator(const std::string& dirPath) : mDirPath(dirPath), mIterator(dirPath) {}

  void Reset() { mIterator = std::filesystem::directory_iterator(mDirPath); }

  void SetDirectory(const std::string& dirPath)
  {
    if (dirPath != mDirPath)
    {
      mDirPath = dirPath;
      Reset();
    }
  }

  std::filesystem::path GetNextFilePath()
  {
    while (mIterator != std::filesystem::end(mIterator))
    {
      if (mIterator->is_regular_file())
      {
        const auto path = mIterator->path();
        ++mIterator;
        return path;
      }
      ++mIterator;
    }
    return {};
  }
};
