//
// Wrapper to libinput.h
// 
#pragma once
#include <memory>
#include <iosfwd>
#include <atomic>
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
    std::string sysname;
  };

  struct PointerMotionEvent : Event {
    float x, y;   // Normal
    float ux, uy; // Unaccelerated
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

  struct DeviceEvent : Event {
    enum Type : int {
        kUnknown_Type  = -1,
        kNone_Type     = 0,
        kPointer_Type  = 1 << 0,
        kKeyboard_Type = 1 << 1,
    };

    int type = kNone_Type;
  };
  class LibInput {
  public:
    ~LibInput() {}

    void startWaitEvents();
    void stopWaitEvents();
    std::function<void(DeviceEvent)> onDeviceAdded;
    std::function<void(DeviceEvent)> onDeviceRemoved;
    std::function<void(PointerMotionEvent)> onPointerMotion;
    std::function<void(PointerButtonEvent)> onPointerButton;

    static std::unique_ptr<LibInput> MakeFromUDev();
#if 0
    static std::unique_ptr<LibInput> MakeFromPaths(const std::vector<std::string>& paths);
#endif
    static std::ostream& log;
  private:
    struct Deleter {
      void operator()(libinput* ptr) const;
    };
    typedef std::unique_ptr<libinput, Deleter> Ptr;
    LibInput(Ptr&& li) :
      m_li{std::move(li)} {}

#if 0
    static bool add_device_from_path(Ptr& li, const std::string& path);
#endif
    void nextEvent();
    void processEvent(libinput_event* ev);
    void handleDefaultEvent(libinput_event* li_ev, Event& ev);

    Ptr m_li;
    std::atomic_bool m_shouldStop; // FIXME: Debería ser atómico?
  };
} // namespace li
