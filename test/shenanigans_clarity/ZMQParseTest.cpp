#include "ZMQUtils.hpp"

TEST(ZMQParseTest, String)
{
  const auto input = "Chumbawamba";
  zmq::message_t message;
  {
    const std::string str = input;
    message = CreateMessage(str);
  }
  ASSERT_FALSE(message.empty());
  ASSERT_EQ(message.size(), sizeof("Chumbawamba") - 1);
  const auto output = ParseMessage<std::string_view>(message);
  ASSERT_EQ(output, input);
}

TEST(ZMQParseTest, Mat)
{
  const auto input = "Chumbawamba";
  zmq::message_t message;
  {
    const std::string str = input;
    message = CreateMessage(str);
  }
  ASSERT_FALSE(message.empty());
  ASSERT_EQ(message.size(), sizeof("Chumbawamba") - 1);
  const auto output = ParseMessage<std::string_view>(message);
  ASSERT_EQ(output, input);
}
