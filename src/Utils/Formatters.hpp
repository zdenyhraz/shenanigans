#pragma once

template <typename T>
struct fmt::formatter<cv::Point_<T>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  constexpr auto format(const cv::Point_<T>& point, FormatContext& ctx) const
  {
    return fmt::format_to(ctx.out(), "[{}, {}]", point.x, point.y);
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
  constexpr auto format(const cv::Size& size, FormatContext& ctx) const
  {
    return fmt::format_to(ctx.out(), "[{}, {}]", size.width, size.height);
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
  constexpr auto format(const cv::Mat& mat, FormatContext& ctx) const
  {
    if (mat.rows == 0)
      return fmt::format_to(ctx.out(), "[]");

    fmt::format_to(ctx.out(), "[");
    for (int r = 0; r < mat.rows; ++r)
    {
      for (int c = 0; c < mat.cols; ++c)
        fmt::format_to(ctx.out(), "{}, ", mat.at<float>(r, c));
      fmt::format_to(ctx.out(), "\n");
    }

    return fmt::format_to(ctx.out(), "]");
  }
};
