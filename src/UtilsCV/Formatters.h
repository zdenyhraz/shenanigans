#pragma once

template <>
struct fmt::formatter<cv::Point2f>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  constexpr auto format(const cv::Point2f& point, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "[{:.2f}, {:.2f}]", point.x, point.y);
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