#include "mice.h"

#include <cstring>

#define safe_call(ev, args) do {if(ev) ev(args);} while(0)

std::shared_ptr<Mice> Mice::MakeFromSystem() {
   auto lib = li::LibInput::MakeFromUDev();
   if(!lib)
      return nullptr;
   
   return std::shared_ptr<Mice>(new Mice(std::move(lib)));
}

Mice::Mice(input_t&& input) :
   m_input{input} {
   m_input->onDeviceRemoved = [this] (auto ev) {this->onDeviceRemoved(ev);};
   m_input->onDeviceAdded   = [this] (auto ev) {this->onDeviceAdded(ev);};
   m_input->onPointerMotion = [this] (auto ev) {this->onPointerMotionEvent(ev);};
   m_input->onPointerButton = [this] (auto ev) {this->onPointerButtonEvent(ev);};
}

void Mice::startEventHandling() {
   m_input->startWaitEvents();
}

void Mice::stopEventHandling() {
   m_input->stopWaitEvents();
}

void Mice::onDeviceAdded(li::DeviceEvent ev) {
   if(ev.type == li::DeviceEvent::kPointer_Type)
      m_mice[ev.sysname] = {};
}

void Mice::onDeviceRemoved(li::DeviceEvent ev) {
   m_mice.erase(ev.sysname);
}

void Mice::onPointerMotionEvent(li::PointerMotionEvent ev) {
   if(m_mice.find(ev.sysname) == m_mice.end())
      return;

   Mouse& mouse = m_mice[ev.sysname];
   mouse.x += ev.x;
   mouse.y += ev.y;
   // m_mice[ev.sysname].rel_x = ev.x;
   // m_mice[ev.sysname].rel_y = ev.y;
   // m_mice[ev.sysname].rel_ux = ev.ux;
   // m_mice[ev.sysname].rel_uy = ev.ux;

   safe_call(onEvent, mouse);
}

void Mice::onPointerButtonEvent(li::PointerButtonEvent ev) {
   if(m_mice.find(ev.sysname) == m_mice.end())
      return;
      
   Mouse& mouse = m_mice[ev.sysname];
   mouse.button = ev.button;
   mouse.button_state = ev.state;

   safe_call(onEvent, mouse);
}