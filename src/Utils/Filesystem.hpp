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

inline bool IsImagePath(const std::string& path)
{
  return path.ends_with(".png") or path.ends_with(".PNG") or path.ends_with(".jpg") or path.ends_with(".JPG") or path.ends_with(".jpeg") or path.ends_with(".JPEG");
}

inline usize GetFileCount(const std::filesystem::path& dirpath)
{
  usize count = 0;
  for (const auto& entry : std::filesystem::directory_iterator(dirpath))
    ++count;
  return count;
}
