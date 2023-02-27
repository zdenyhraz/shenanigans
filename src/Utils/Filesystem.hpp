#pragma once

inline std::filesystem::path GetProjectDirectoryPath(std::string_view relpath = "")
{
  std::filesystem::path path = std::filesystem::current_path();
  for (usize trial = 0; trial < 10; ++trial)
  {
    if (std::filesystem::exists(path / "src"))
      return relpath.empty() ? path : path / relpath;
    path = path.parent_path();
  }
  throw std::runtime_error("Could not find root project directory");
}

inline usize GetFileCount(const std::filesystem::path& dirpath)
{
  if (not std::filesystem::is_directory(dirpath))
    throw std::invalid_argument(fmt::format("{} is not a valid directory", dirpath.string()));
  return std::ranges::count_if(std::filesystem::directory_iterator(dirpath), [](const auto& entry) { return entry.is_regular_file(); });
}
