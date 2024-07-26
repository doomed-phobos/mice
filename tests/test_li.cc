#include "li.h"
#include "mice.h"

#include <print>
#include <thread>
using namespace std::chrono;

int main() {
  auto lib = li::LibInput::MakeFromUDev();
  if(!lib) {
    return 1;
  }
  lib->onPointerMotion = [](li::PointerMotionEvent ev) {
    std::println("{}: {} {} {} {}", ev.sysname, ev.x, ev.y, ev.ux, ev.uy);
  };
  lib->onPointerButton = [](li::PointerButtonEvent ev) {
    std::println("{}: {} {}", ev.sysname, (int)ev.button, (int)ev.state);
  };

  lib->onDeviceAdded = [](li::DeviceEvent ev) {
    std::println("{}: {}", ev.sysname, ev.type);
  };

  std::thread t([&lib]() {lib->startWaitEvents();});
  std::this_thread::sleep_for(5s);
  lib->stopWaitEvents();

  t.join();
  return 0;
}