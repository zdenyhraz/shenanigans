// #include "ZMQUtils.hpp"

class ZMQSendReceiveTest : public ::testing::Test
{
protected:
  ZMQSendReceiveTest() {}

  void SetUp() override
  try
  {
    const auto port = ports++;
    contextpub = zmq::context_t(1);
    const auto publisherAddress = fmt::format("{}://localhost:{}", transport, port);
    publisher = zmq::socket_t(contextpub, zmq::socket_type::pub);
    publisher.connect(publisherAddress);
    publisher.setsockopt(ZMQ_LINGER, 0);
    LOG_DEBUG("Publisher connected to address {}", publisherAddress);
    std::this_thread::sleep_for(std::chrono::seconds(3)); // wait for the connection to be established

    const auto subscriberAddress = fmt::format("{}://*:{}", transport, port);
    static constexpr auto timeout = static_cast<int>(std::chrono::milliseconds(3s).count());
    contextsub = zmq::context_t(1);
    subscriber = zmq::socket_t(contextsub, zmq::socket_type::sub);
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    subscriber.setsockopt(ZMQ_RCVTIMEO, timeout); // add timeout to allow for periodic shutdownRequested checking
    subscriber.setsockopt(ZMQ_LINGER, 0);
    subscriber.bind(subscriberAddress);
    LOG_DEBUG("Subscriber connected to address {}", subscriberAddress);
    std::this_thread::sleep_for(std::chrono::seconds(3)); // wait for the connection to be established
  }
  catch (const std::exception& e)
  {
    LOG_EXCEPTION(e);
    throw;
  }

  static constexpr std::string_view transport = IsPOSIX() ? "ipc" : "tcp"; // ipc not available on windows
  inline static std::atomic<int> ports = 5555;
  zmq::socket_t publisher;
  zmq::socket_t subscriber;
  zmq::context_t contextpub;
  zmq::context_t contextsub;
};

// TEST_F(ZMQSendReceiveTest, SendReceiveString)
// {
//   const std::string str = "SendReceiveString";
//   publisher.send(EncodeMessage(str));
//   zmq::message_t message;
//   LOG_DEBUG("Receiving message");
//   subscriber.recv(&message);
//   ASSERT_FALSE(message.empty());
//   ASSERT_EQ(DecodeMessage<std::string_view>(message), str);
// }
