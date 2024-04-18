#include "ZMQUtils.hpp"

TEST(ZMQEncodeDecodeTest, String)
{
  const auto target = "Chumbawamba";
  zmq::message_t message;
  {
    const std::string input = target;
    message = EncodeMessage(input);
  }
  ASSERT_FALSE(message.empty());
  const auto output = DecodeMessage<std::string_view>(message);
  ASSERT_EQ(output, target);
}

TEST(ZMQEncodeDecodeTest, Mat)
{
  const auto target = cv::imread(GetProjectDirectoryPath("test/shenanigans/data/baboon.png").string());
  zmq::message_t message;
  {
    const auto input = target.clone();
    message = EncodeMessage(input);
  }
  ASSERT_FALSE(message.empty());
  const auto output = DecodeMessage(message, target.rows, target.cols, target.type());
  ASSERT_TRUE(output.isContinuous());
  ASSERT_EQ(output.rows, target.rows);
  ASSERT_EQ(output.cols, target.cols);
  ASSERT_EQ(output.type(), target.type());
  ASSERT_EQ(output.depth(), target.depth());
  ASSERT_EQ(cv::norm(output, target, cv::NORM_L1), 0);
}

TEST(ZMQEncodeDecodeTest, Vector)
{
  struct S
  {
    int a;
    int b;
    float c;

    auto operator<=>(const S&) const = default;
  };

  const std::vector<S> target = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  zmq::message_t message;
  {
    const auto input = target;
    message = EncodeMessage(input);
  }
  ASSERT_FALSE(message.empty());
  const auto output = DecodeMessage<std::span<S>>(message);
  ASSERT_EQ(output.size(), target.size());
  ASSERT_NE(output.data(), target.data());
  for (usize i = 0; i < output.size(); ++i)
    ASSERT_EQ(output[i], target[i]);
}

TEST(ZMQEncodeDecodeTest, Span)
{
  struct S
  {
    int a;
    int b;
    float c;

    auto operator<=>(const S&) const = default;
  };

  std::vector<S> target = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  zmq::message_t message;
  {
    const std::span<S> input = target;
    message = EncodeMessage(input);
  }
  ASSERT_FALSE(message.empty());
  const auto output = DecodeMessage<std::span<S>>(message);
  ASSERT_EQ(output.size(), target.size());
  ASSERT_NE(output.data(), target.data());
  for (usize i = 0; i < output.size(); ++i)
    ASSERT_EQ(output[i], target[i]);
}

TEST(ZMQEncodeDecodeTest, Raw)
{
  struct S
  {
    int a;
    int b;
    float c;
    double d;
    long long e;

    auto operator<=>(const S&) const = default;
  };

  const S target{1, 2, 3, 4, 5};
  zmq::message_t message;
  {
    const S input = target;
    message = EncodeMessageRaw(input);
  }
  ASSERT_FALSE(message.empty());
  ASSERT_EQ(message.size(), sizeof(S));
  const auto& output = DecodeMessageRaw<S>(message);
  ASSERT_EQ(output, target);
}
