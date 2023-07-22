#pragma once
#include "li.h"

#include <unordered_map>

class Mice {
public:
   struct Mouse {
      Mouse() :
         rel_x{0}, rel_y{0},
         rel_ux{0}, rel_uy{0},
         button{li::PointerButtonEvent::kNone_Button},
         button_state{li::PointerButtonEvent::kNone_State} {}

      double rel_x, rel_y;
      double rel_ux, rel_uy;
      li::PointerButtonEvent::Button button;
      li::PointerButtonEvent::State button_state;
   };

   typedef std::shared_ptr<li::LibInput> input_t;
   typedef std::unordered_map<std::string, Mouse> map_t;

   void startEventHandling();
   void stopEventHandling();

   // sysname - properties of mouse
   const map_t& miceMap() const {return m_mice;}

   static std::shared_ptr<Mice> MakeFromSystem();
private:
   Mice(input_t&& input);

   void onPointerMotionEvent(li::PointerMotionEvent ev);
   void onPointerButtonEvent(li::PointerButtonEvent ev);

   map_t m_mice;
   input_t m_input;
};