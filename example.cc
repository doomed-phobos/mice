#include "mice.h"
#include <iostream>
#include <thread>
using namespace std;

int main() {
   auto mice = Mice::MakeFromSystem();
   if(!mice)
      return 1;

   mice->waitEvents();

   return 0;
}