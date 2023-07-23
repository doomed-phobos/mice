#include "mice.h"
#include <iostream>
using namespace std;

auto mice = Mice::MakeFromSystem();

int main() {
   if(!mice)
      return 1;
   mice->onEvent = [](const Mice::Mouse& m) {
      printf("%.2f,%.2f\n", m.x, m.y);
   };

   mice->startEventHandling();

   return 0;
}