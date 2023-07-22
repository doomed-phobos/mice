#include "mice.h"

#include <cstring>

#define DEVICES_PATH "/proc/bus/input/devices"
#define safe_call(ev, args) if(ev) ev(args)

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
   Mouse& mouse = m_mice[ev.sysname];
   
   mouse.sysname = ev.sysname;
   mouse.rel_x = ev.x;
   mouse.rel_y = ev.y;
   mouse.rel_ux = ev.ux;
   mouse.rel_uy = ev.ux;

   safe_call(onEvent, mouse);
}

void Mice::onPointerButtonEvent(li::PointerButtonEvent ev) {
   Mouse& mouse = m_mice[ev.sysname];
   m_mice[ev.sysname].button = ev.button;
   m_mice[ev.sysname].button_state = ev.state;
   mouse.sysname = ev.sysname;

   safe_call(onEvent, mouse);
}