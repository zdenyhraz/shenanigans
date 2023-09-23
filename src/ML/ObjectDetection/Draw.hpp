#pragma once

void DrawPrediction(cv::Mat& frame, cv::Rect rect, std::string_view classname, float confidence)
{
  const auto boxThickness = std::clamp(0.007 * frame.rows, 1., 100.);
  const auto fontThickness = std::clamp(0.003 * frame.rows, 1., 100.);
  const auto multiplier = frame.type() == CV_8UC3 ? 255. : 1.;
  const auto color = multiplier * cv::Scalar(Random::Rand(0.4, 1), Random::Rand(0.4, 1), Random::Rand(0.4, 1));
  const f64 fontScale = 1.5 * frame.rows / 1600;
  const auto font = cv::FONT_HERSHEY_SIMPLEX;
  const f64 labelPaddingX = 0.2;
  const f64 labelPaddingY = 0.5;

  cv::rectangle(frame, rect, color, boxThickness);

  std::string label = fmt::format("{}: {:.1f}%", classname, confidence * 100);
  cv::Size labelSize = cv::getTextSize(label, font, fontScale, fontThickness, nullptr);
  cv::rectangle(frame, cv::Point(rect.tl().x - 0.5 * boxThickness, rect.tl().y - labelSize.height * (1. + labelPaddingY)),
      cv::Point(rect.tl().x + labelSize.width * (1. + labelPaddingX), rect.tl().y), color, cv::FILLED);
  cv::putText(frame, label, cv::Point(rect.tl().x + 0.5 * labelPaddingX * labelSize.width, rect.tl().y - 0.5 * labelPaddingY * labelSize.height), font, fontScale,
      cv::Scalar::all(0), fontThickness, cv::LINE_AA);
  LOG_DEBUG("Detection: {} ({:.1f}%) xywh: [{},{},{},{}]", classname, confidence * 100, rect.x, rect.y, rect.width, rect.height);
}
