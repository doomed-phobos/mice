#pragma once
#include "li.h"

#include <unordered_map>

class Mice {
public:
   struct Mouse {
      Mouse() :
         x{0}, y{0} {}
      double x, y;
   };

   typedef std::shared_ptr<li::LibInput> input_t;
   typedef std::unordered_map<std::string, Mouse> map_t;

   void waitEvents();

   // sysname - properties of mouse
   const map_t& miceMap() const {return m_mice;}

   static std::shared_ptr<Mice> MakeFromSystem();
private:
   Mice(input_t&& input);

   void onEvent(li::MotionEvent ev);

   map_t m_mice;
   input_t m_input;
};