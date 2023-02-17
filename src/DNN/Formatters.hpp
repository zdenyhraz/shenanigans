#pragma once

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
