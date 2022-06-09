#pragma once
#include <thread>
#include <mutex>
#include <string>
#include <sstream>
#include <functional>

std::string GetCurrentThreadId()
{
  std::stringstream ss;
  ss << std::this_thread::get_id();
  const auto str = ss.str();
  return fmt::format("on thread x{}", str.c_str() + str.size() - 4);
}

class ThreadLoop
{
public:
  ThreadLoop(const std::string& name) : mName(name) {}
  ~ThreadLoop() { Stop(); }

  template <typename F>
  void Start(F&& function)
  {
    if (mRunning)
      return;

    std::scoped_lock lock(mMutex);
    if (mThread.joinable())
      mThread.join();

    mRunning = true;
    mThread = std::jthread(
        [this, function]()
        {
          fmt::print("Thread '{}' started {}\n", mName, GetCurrentThreadId());
          while (mRunning)
          {
            function();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }
          fmt::print("Thread '{}' stopped {}\n", mName, GetCurrentThreadId());
        });
  }

  void Stop()
  {
    if (not mRunning)
      return;

    mRunning = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

private:
  std::atomic<bool> mRunning = false;
  std::mutex mMutex;
  std::jthread mThread;
  std::string mName;
};
