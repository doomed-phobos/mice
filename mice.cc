#include "mice.h"

#include <cstring>

#define DEVICES_PATH "/proc/bus/input/devices"

namespace {
   struct FILEDeleter {
      void operator()(FILE* f) {
         if(f)
            fclose(f);
      }
   };

   typedef std::unique_ptr<FILE, FILEDeleter> FILEPtr;
}

std::shared_ptr<Mice> Mice::MakeFromSystem() {
   auto lib = li::LibInput::Make();
   if(!lib)
      return nullptr;
      
   FILEPtr file(fopen(DEVICES_PATH, "r"));
   if(!file) {
      fprintf(stderr, "Failed to open '%s':%s\n", DEVICES_PATH, strerror(errno));
      return nullptr;
   }

   Mice* mice = new Mice(std::move(lib));
   char buf[50];
   while(fgets(buf, 50, file.get()) != nullptr) {
      if(strncmp(buf, "H: Handlers=mouse", 17) != 0)
         continue;

      char* ptr = buf+17;
      ptr = strchr(ptr, ' ')+1;
      char* end = strchr(ptr, ' ');
      std::string sysname(ptr, end);
      if(!mice->m_input->addDeviceFromPath("/dev/input/" + sysname))
         return nullptr;

      mice->m_mice[sysname] = {};
   }

   return std::shared_ptr<Mice>(mice);
}

Mice::Mice(input_t&& input) :
   m_input{input} {
   m_input->onMotion = [this] (li::MotionEvent ev) {this->onEvent(ev);};
}

void Mice::waitEvents() {
   m_input->waitEvents();
}

void Mice::onEvent(li::MotionEvent ev) {
   m_mice[ev.sysname].x += ev.x;
   m_mice[ev.sysname].y += ev.y;
}