#include "li.h"

#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <libinput.h>
#include <linux/input.h>

#define safe_call(ev, args) if(ev) ev(args)

namespace li {
   const libinput_interface& LibInterface::GetInterface() {
      static libinput_interface interface {
         .open_restricted = open_restricted,
         .close_restricted = close_restricted,
      };

      return interface;
   }

   int LibInterface::open_restricted(const char* path, int flags, void* /*user_data*/) {
      int fd = open(path, flags);
      return fd < 0 ? -errno : fd;
   }

   void LibInterface::close_restricted(int fd, void* /*user_data*/) {
      close(fd);
   }

   std::shared_ptr<LibInput> LibInput::Make() {
      libinput* li;

      li = libinput_path_create_context(&LibInterface::GetInterface(), nullptr);
      if(!li) {
         perror("Failed to create context");
         return nullptr;
      }

      return std::shared_ptr<LibInput>(new LibInput(li));
   }

   bool LibInput::addDeviceFromPath(const std::string& path) {
      if(path.empty())
         return false;
      
      libinput_device* device;

      device = libinput_path_add_device(m_li, path.c_str());
      if(!device) {
         fprintf(stderr, "Failed to open '%s': %s\n", path.c_str(), strerror(errno));
         return false;
      }

      return true;
   }

   LibInput::~LibInput() {
      libinput_unref(m_li);
      m_li = nullptr;
   }

   void LibInput::startWaitEvents() {
      pollfd fds {
         .fd = libinput_get_fd(m_li),
         .events = POLLIN,
         .revents = 0,
      };
      
      m_stop = false;
      do {
         nextEvent();
      } while(!m_stop && poll(&fds, 1, -1) > -1);
   }

   void LibInput::stopWaitEvents() {
      m_stop = true;
   }

   void LibInput::nextEvent() {
      libinput_event* ev;

      libinput_dispatch(m_li);
      while((ev = libinput_get_event(m_li))) {
         handleEvents(ev);

         libinput_event_destroy(ev);
      }
   }

   void LibInput::handleEvents(libinput_event* ev) {
      switch(libinput_event_get_type(ev)) {
         case LIBINPUT_EVENT_POINTER_MOTION: {
            auto p = libinput_event_get_pointer_event(ev);
            PointerMotionEvent mev;
            handleDefaultEvent(ev, mev);
            
            mev.x = libinput_event_pointer_get_dx(p);
            mev.y = libinput_event_pointer_get_dy(p);
            mev.ux = libinput_event_pointer_get_dx_unaccelerated(p);
            mev.uy = libinput_event_pointer_get_dy_unaccelerated(p);

            safe_call(onPointerMotion, mev);
         }
         break;
         case LIBINPUT_EVENT_POINTER_BUTTON: {
            auto p = libinput_event_get_pointer_event(ev);
            PointerButtonEvent bev;
            handleDefaultEvent(ev, bev);

            uint32_t button = libinput_event_pointer_get_button(p);
            bev.button = (button == BTN_LEFT ? PointerButtonEvent::kLeft_Button : 
                          button == BTN_MIDDLE ? PointerButtonEvent::kMiddle_Button :
                          button == BTN_RIGHT ? PointerButtonEvent::kRight_Button :
                          PointerButtonEvent::kUnknown_Button);
            bev.state = (libinput_event_pointer_get_button_state(p) == LIBINPUT_BUTTON_STATE_PRESSED ?
                         PointerButtonEvent::kPressed_State : PointerButtonEvent::kReleased_State);
            safe_call(onPointerButton, bev);    
         }
         break;
      }   
   }

   void LibInput::handleDefaultEvent(libinput_event* li_ev, Event& ev) {
      auto device = libinput_event_get_device(li_ev);
      ev.sysname = libinput_device_get_sysname(device);
   }
} // namespace li