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

template <typename T>
struct fmt::formatter<cv::Point_<T>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  constexpr auto format(const cv::Point_<T>& point, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "[{:.3f}, {:.3f}]", point.x, point.y);
  }
};

template <>
struct fmt::formatter<cv::Size>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  constexpr auto format(const cv::Size& size, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "[{}, {}]", size.width, size.height);
  }
};

template <>
struct fmt::formatter<cv::MatSize>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  constexpr auto format(const cv::MatSize& size, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "{}", size());
  }
};

template <>
struct fmt::formatter<cv::Mat>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  constexpr auto format(const cv::Mat& mat, FormatContext& ctx)
  {
    if (mat.rows == 0)
      return fmt::format_to(ctx.out(), "[]");

    fmt::format_to(ctx.out(), "[");
    for (i32 r = 0; r < mat.rows; ++r)
    {
      for (i32 c = 0; c < mat.cols; ++c)
        fmt::format_to(ctx.out(), "{}, ", mat.at<f32>(r, c));
      fmt::format_to(ctx.out(), "\n");
    }

    return fmt::format_to(ctx.out(), "]");
  }
};

template <>
struct fmt::formatter<c10::IntArrayRef>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  constexpr auto format(const c10::IntArrayRef& arr, FormatContext& ctx)
  {
    fmt::format_to(ctx.out(), "[");
    for (usize dim = 0; dim < arr.size(); ++dim)
      fmt::format_to(ctx.out(), "{}, ", arr[dim]);
    return fmt::format_to(ctx.out(), "]");
  }
};
