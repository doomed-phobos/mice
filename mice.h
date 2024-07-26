#pragma once
#include "li.h"

#include <unordered_map>
#include <functional>

class Mice {
public:
   struct Mouse {
      Mouse() :
         x{0}, y{0},
         button{li::PointerButtonEvent::kNone_Button},
         button_state{li::PointerButtonEvent::kNone_State} {}

      float x, y;
      li::PointerButtonEvent::Button button;
      li::PointerButtonEvent::State button_state;
   };

   typedef std::shared_ptr<li::LibInput> input_t;
   typedef std::unordered_map<std::string_view, Mouse> map_t;

   void startEventHandling();
   void stopEventHandling();

   std::function<void(const Mouse&)> onEvent;

   // sysname - properties of mouse
   const map_t& miceMap() const {return m_mice;}

   static std::shared_ptr<Mice> MakeFromSystem();
private:
   Mice(input_t&& input);

   void onDeviceAdded(li::DeviceEvent ev);
   void onDeviceRemoved(li::DeviceEvent ev);
   void onPointerMotionEvent(li::PointerMotionEvent ev);
   void onPointerButtonEvent(li::PointerButtonEvent ev);

   map_t m_mice;
   input_t m_input;
};