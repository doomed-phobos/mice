#include "mice.h"
#include <iostream>
using namespace std;

auto mice = Mice::MakeFromSystem();

int main() {
   if(!mice)
      return 1;
   
   mice->startEventHandling();

   return 0;
}