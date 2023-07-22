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
   m_input->onPointerMotion = [this] (auto ev) {this->onPointerMotionEvent(ev);};
   m_input->onPointerButton = [this] (auto ev) {this->onPointerButtonEvent(ev);};
}

void Mice::startEventHandling() {
   m_input->startWaitEvents();
}

void Mice::stopEventHandling() {
   m_input->stopWaitEvents();
}

void Mice::onPointerMotionEvent(li::PointerMotionEvent ev) {
   m_mice[ev.sysname].rel_x = ev.x;
   m_mice[ev.sysname].rel_y = ev.y;
   m_mice[ev.sysname].rel_ux = ev.ux;
   m_mice[ev.sysname].rel_uy = ev.ux;
}

void Mice::onPointerButtonEvent(li::PointerButtonEvent ev) {
   m_mice[ev.sysname].button = ev.button;
   m_mice[ev.sysname].button_state = ev.state;
}