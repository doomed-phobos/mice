#include "li.h"

#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <linux/input.h>
#include <libinput.h>
#include <libudev.h>

#define SEATNAME "seat0"
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

   std::shared_ptr<LibInput> LibInput::MakeFromPaths(const std::vector<std::string>& paths) {
      libinput* li;

      li = libinput_path_create_context(&LibInterface::GetInterface(), nullptr);
      if(!li) {
         perror("Failed to create context");
         return nullptr;
      }

      std::shared_ptr<LibInput> lib(new LibInput(li));
      for(const auto& path : paths)
         if(!add_device_from_path(li, path))
            return nullptr;

      return std::move(lib);
   }

   std::shared_ptr<LibInput> LibInput::MakeFromUDev() {
      libinput* li;
      udev* uv = udev_new();
      if(!uv) {
         perror("Failed to initialize udev");
         return nullptr;
      }

      li = libinput_udev_create_context(&LibInterface::GetInterface(), nullptr, uv);
      if(!li) {
         perror("Failed to initialize libinput context from udev");
         return nullptr;
      }

      std::shared_ptr<LibInput> lib(new LibInput(li));
      if(libinput_udev_assign_seat(li, SEATNAME)) {
         perror("Failed to set seat");
         udev_unref(uv);
         return nullptr;
      }

      udev_unref(uv);
      return std::move(lib);
   }

   bool LibInput::add_device_from_path(libinput* li, const std::string& path) {
      if(path.empty() || !li)
         return false;
      
      libinput_device* device;

      device = libinput_path_add_device(li, path.c_str());
      if(!device) {
         return false;
         fprintf(stderr, "Failed to open '%s': %s\n", path.c_str(), strerror(errno));
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
         case LIBINPUT_EVENT_DEVICE_ADDED:
         case LIBINPUT_EVENT_DEVICE_REMOVED: {
            libinput_device* dev = libinput_event_get_device(ev);
            DeviceEvent d_ev;
            bool hasCapability = false;
            handleDefaultEvent(ev, d_ev);

            if(libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_KEYBOARD))
               d_ev.type |= DeviceEvent::kKeyboard_Type, hasCapability = true;
            if(libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_POINTER))
               d_ev.type |= DeviceEvent::kPointer_Type, hasCapability = true;
            
            if(!hasCapability)
               d_ev.type = DeviceEvent::kUnknown_Type;

            if(libinput_event_get_type(ev) == LIBINPUT_EVENT_DEVICE_ADDED)
               safe_call(onDeviceAdded, d_ev);
            else
               safe_call(onDeviceRemoved, d_ev);
         }
         break;
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