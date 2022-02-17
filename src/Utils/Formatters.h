#pragma once

template <typename T>
struct fmt::formatter<std::vector<T>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  constexpr auto format(const std::vector<T>& vec, FormatContext& ctx)
  {
    if (vec.empty())
      return fmt::format_to(ctx.out(), "[]");

    fmt::format_to(ctx.out(), "[");
    for (usize i = 0; i < vec.size() - 1; ++i)
      fmt::format_to(ctx.out(), "{}, ", vec[i]);
    return fmt::format_to(ctx.out(), "{}]", vec[vec.size() - 1]);
  }
};

template <typename T>
inline std::string to_string(const std::vector<T>& vec)
{
  std::stringstream out;
  out << "[";
  for (usize i = 0; i < vec.size(); i++)
  {
    out << vec[i];
    if (i < vec.size() - 1)
      out << ", ";
  }
  out << "]";
  return out.str();
}

template <typename T>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec)
{
  out << "[";
  for (usize i = 0; i < vec.size(); i++)
  {
    out << vec[i];
    if (i < vec.size() - 1)
      out << ", ";
  }
  out << "]";
  return out;
}

template <typename T>
inline std::ostream& operator<<(std::ostream& out, const std::vector<std::vector<T>>& vec)
{
  for (i32 r = 0; r < vec.size(); r++)
  {
    out << "[";
    for (i32 c = 0; c < vec[r].size(); c++)
    {
      out << vec[r][c];
      if (c < vec[r].size() - 1)
        out << ", ";
    }
    out << "]\n";
  }
  return out;
}

inline std::ostream& operator<<(std::ostream& out, const std::vector<std::string>& vec)
{
  for (usize i = 0; i < vec.size(); i++)
    out << vec[i] << "\n";
  return out;
}