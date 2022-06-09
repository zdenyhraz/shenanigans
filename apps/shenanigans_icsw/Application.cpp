#include <fmt/format.h>
#include "Instrument.hpp"
#include "ThreadUtils.hpp"

int main(int argc, char** argv)
{
  fmt::print("ICSW started {}\n", GetCurrentThreadId());
  std::srand(std::time(nullptr));

  Instrument instrument;
  std::this_thread::sleep_for(std::chrono::seconds(1));

  fmt::print("ICSW stopped {}\n", GetCurrentThreadId());
  return EXIT_SUCCESS;
}
