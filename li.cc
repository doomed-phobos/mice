#include "li.h"

// #include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <libinput.h>
// #include <libudev.h>

#define SEATNAME "seat0" // TODO: Cambiar esto?

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

  std::ostream& LibInput::log = std::cout;

  void LibInput::Deleter::operator()(libinput* ptr) const {
    libinput_unref(ptr);
  }
  
  void LibInput::startWaitEvents() {
    pollfd fds {
      .fd = libinput_get_fd(m_li.get()),
      .events = POLLIN,
      .revents = 0,
    };

    m_shouldStop = false;
    do {
      nextEvent();
    } while(!m_shouldStop && poll(&fds, 1, 5000) > 0);
  }

  void LibInput::stopWaitEvents() {
    m_shouldStop = true;
  }

  void LibInput::nextEvent() {
    libinput_dispatch(m_li.get());
    for(libinput_event* ev; (ev = libinput_get_event(m_li.get())); libinput_event_destroy(ev)) {
      processEvent(ev);
    }
  }
  
  void LibInput::processEvent(libinput_event* ev) {
  }

  std::unique_ptr<LibInput> LibInput::MakeFromUDev() {
    struct Udev {
        udev* ptr;
        
        Udev() :
          ptr{udev_new()} {}

        ~Udev() {
          udev_unref(ptr);
        }

        operator udev*() {return ptr;}
    } uv;
    
    if(!uv) {
      log << "Failed to initialize udev";
      return nullptr;
    }
    
    Ptr li(libinput_udev_create_context(&LibInterface::GetInterface(), nullptr, uv));
    if(!li) {
      log << "Failed to initialize libinput context from udev";
      return nullptr;
    }

    if(libinput_udev_assign_seat(li.get(), SEATNAME)) {
      log << "Failed to set seat";
      return nullptr;
    }

    return std::unique_ptr<LibInput>{new LibInput(std::move(li))};
  }

#if 0
  std::unique_ptr<LibInput> LibInput::MakeFromPaths(const std::vector<std::string>& paths) {
    Ptr li(libinput_path_create_context(&LibInterface::GetInterface(), nullptr));
    if(!li) {
        log << "Failed to create context";
        return nullptr;
    }

    // std::unique_ptr<LibInput> lib(new LibInput(li));
    for(const auto& path : paths)
        if(!add_device_from_path(li, path))
          return nullptr;

    return lib;
  }

  bool LibInput::add_device_from_path(Ptr& li, const std::string& path) {
    libinput_device* device;

    libinput_path_add_device(struct libinput *libinput, const char *path)
    device = libinput_path_add_device(li, path.c_str());
    if(!device) {
        return false;
        fprintf(stderr, "Failed to open '%s': %s\n", path.c_str(), strerror(errno));
    }

    return true;
  }
#endif
/*
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
          
          mev.x = static_cast<float>(libinput_event_pointer_get_dx(p));
          mev.y = static_cast<float>(libinput_event_pointer_get_dy(p));
          mev.ux = static_cast<float>(libinput_event_pointer_get_dx_unaccelerated(p));
          mev.uy = static_cast<float>(libinput_event_pointer_get_dy_unaccelerated(p));

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
  }*/
} // namespace li