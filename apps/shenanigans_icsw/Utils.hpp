#pragma once
#include <thread>
#include <mutex>
#include <string>
#include <sstream>
#include <fmt/format.h>

std::string GetCurrentThreadId()
{
  std::stringstream ss;
  ss << std::this_thread::get_id();
  const auto str = ss.str();
  return fmt::format("on thread x{}", str.c_str() + str.size() - 4);
}

template <typename Fmt, typename... Args>
void Log(Fmt&& format, Args&&... args)
{
  fmt::print("{} {}\n", fmt::vformat(std::forward<Fmt>(format), fmt::make_format_args(std::forward<Args>(args)...)), GetCurrentThreadId());
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
          Log("Thread '{}' started", mName);
          while (mRunning)
          {
            function();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
          }
          Log("Thread '{}' stopped", mName);
        });
  }

  void Stop()
  {
    if (not mRunning)
      return;

    mRunning = false;
  }

private:
  std::atomic<bool> mRunning = false;
  std::mutex mMutex;
  std::jthread mThread;
  std::string mName;
};
