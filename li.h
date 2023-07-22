//
// Wrapper to libinput.h
// 
#pragma once
#include <memory>
#include <vector>
#include <functional>

struct libinput;
struct libinput_event;
struct libinput_interface;

namespace li {
   class LibInterface {
   public:
      static const libinput_interface& GetInterface();
   private:
      static int open_restricted(const char* path, int flags, void* user_data);
      static void close_restricted(int fd, void* user_data);
   };

   struct Event {
      const char* sysname;
   };

   struct PointerMotionEvent : Event {
      double x, y;   // Normal
      double ux, uy; // Unaccelerated
   };

   struct PointerButtonEvent : Event {
      enum Button {
         kNone_Button,
         kLeft_Button,
         kMiddle_Button,
         kRight_Button,
         kUnknown_Button,
      } button = kNone_Button;

      enum State {
         kNone_State,
         kPressed_State,
         kReleased_State,
      } state = kNone_State;
   };

   class LibInput {
   public:
      ~LibInput();

      void startWaitEvents();
      void stopWaitEvents();
      std::function<void(PointerMotionEvent)> onPointerMotion;
      std::function<void(PointerButtonEvent)> onPointerButton;

      bool addDeviceFromPath(const std::string& path);

      static std::shared_ptr<LibInput> Make();
   private:
      LibInput(libinput* li) :
         m_li{li} {}

      void nextEvent();
      void handleEvents(libinput_event* ev);
      void handleDefaultEvent(libinput_event* li_ev, Event& ev);

      libinput* m_li;
      bool m_stop = false;
   };
} // namespace li