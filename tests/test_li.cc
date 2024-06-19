#include "li.h"

#include <iostream>

int main() {
  auto lib = li::LibInput::MakeFromUDev();
  if(!lib) {
    return 1;
  }

  lib->startWaitEvents();
  // lib->stopWaitEvents();

  return 0;
}