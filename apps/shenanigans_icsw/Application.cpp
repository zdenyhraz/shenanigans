#include <fmt/format.h>
#include "Utils.hpp"
#include "Instrument.hpp"
#include "CommInterface.hpp"

int main(int argc, char** argv)
{
  Log("ICSW started");
  // flawfinder: ignore
  std::srand(std::time(nullptr));

  Instrument instrument;
  CommInterface commInterface(instrument);

  commInterface.StartListening();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  commInterface.StopListening();

  Log("ICSW stopped");
  return EXIT_SUCCESS;
}
