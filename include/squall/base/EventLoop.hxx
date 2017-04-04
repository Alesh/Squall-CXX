#ifndef SQUALL__BASE__EVENT_LOOP__HXX
#define SQUALL__BASE__EVENT_LOOP__HXX
#include <ev.h>
#include <memory>
#include <functional>
#include <unordered_map>

namespace squall {

/**
 * Event codes
 */
enum Event : int {
    Read = EV_READ,
    Write = EV_WRITE,
    Timeout = EV_TIMER,
    Signal = EV_SIGNAL,
    Error = EV_ERROR,
    Cleanup = EV_CLEANUP,
};

namespace base {


/**
 * Base template class that implement an event loop.
 */
template <typename Context>
class EventLoop {
    using OnEvent = std::function<void(Context context, int revents) noexcept>;

    OnEvent _on_event;
    bool _running = false;
    struct ev_loop* _p_loop;
    std::unordered_map<Context, std::unique_ptr<ev_io>> _io_watchers;
    std::unordered_map<Context, std::unique_ptr<ev_timer>> _timer_watchers;
    std::unordered_map<Context, std::unique_ptr<ev_signal>> _signal_watchers;

    template <typename Watcher>
    static void callback(struct ev_loop* p_loop, Watcher* p_watcher, int revents) {
        auto p_event_loop = static_cast<EventLoop<Context>*>(ev_userdata(p_loop));
        auto p_context = static_cast<Context*>(p_watcher->data);
        p_event_loop->_on_event(*p_context, revents);
    }

  public:
    /* Constructor 
     * `on_event` is the callback that will be called to transmit occurred events. */
    EventLoop(const OnEvent& on_event) : _on_event(on_event) {
        _p_loop = ev_loop_new(0);
        ev_set_userdata(_p_loop, static_cast<void*>(this));
    };

    /* Destructor */
    virtual ~EventLoop() {
        ev_loop_destroy(_p_loop);
    }

    /* Return true if an event dispatching is active. */
    bool is_running() {
        return _running;
    }

    /* Starts event dispatching.*/
    void start() {
        _running = true;
        while (_running)
            ev_run(_p_loop, EVRUN_ONCE);
        while (_io_watchers.size())
            cancel_io(_io_watchers.begin()->first);
        while (_timer_watchers.size())
            cancel_timer(_timer_watchers.begin()->first);
        while (_signal_watchers.size())
            cancel_signal(_signal_watchers.begin()->first);
    }

    /* Stops event dispatching.*/
    void stop() {
        if (_running) {
            _running = false;
            ev_break(_p_loop, EVBREAK_ONE);
        }
    }

    /* Setup to call `on_event` for a given `context` 
     * when the I/O device with a given `fd` would be to read and/or write `mode`. */
    bool setup_io(Context context, int fd, int mode) {
        auto p_watcher = new ev_io();
        ev_io_init(p_watcher, EventLoop::template callback<ev_io>, fd, mode);
        ev_io_start(_p_loop, p_watcher);
        if (ev_is_active(p_watcher)) {
            auto result = _io_watchers.insert(std::make_pair(context, std::unique_ptr<ev_io>(p_watcher)));
            if (!result.second) {
                cancel_io(result.first->first);
                result = _io_watchers.insert(std::make_pair(context, std::unique_ptr<ev_io>(p_watcher)));
            }
            if (result.second) {
                result.first->second.get()->data = (void*)(&(result.first->first));
                return true;
            }
        }
        return false;
    }

    /* Updates I/O mode for event watchig extablished with method `setup_io` for a given `context`. */
    bool update_io(Context context, int mode) {
        auto found = _io_watchers.find(context);
        if (found != _io_watchers.end()) {
            auto p_watcher = found->second.get();
            if (_running && ev_is_active(p_watcher))
                ev_io_stop(_p_loop, p_watcher);
            ev_io_set(p_watcher, p_watcher->fd, mode);
            ev_io_start(_p_loop, p_watcher);
            if (ev_is_active(p_watcher))
                return true;
        }
        return false;
    }    

    /* Cancels an event watchig extablished with method `setup_io` for a given `context`. */
    bool cancel_io(Context context) {
        auto found = _io_watchers.find(context);
        if (found != _io_watchers.end()) {
            auto p_watcher = found->second.get();
            if (ev_is_active(p_watcher))
                ev_io_stop(_p_loop, p_watcher);
            _io_watchers.erase(found);
            return true;
        }
        return false;
    }

    /* Setup to call `on_event` for a given `context` every `seconds`. */
    bool setup_timer(Context context, double seconds) {
        auto p_watcher = new ev_timer();
        ev_timer_init(p_watcher, EventLoop::template callback<ev_timer>, seconds, seconds);
        ev_timer_start(_p_loop, p_watcher);
        if (ev_is_active(p_watcher)) {
            auto result = _timer_watchers.insert(std::make_pair(context, std::unique_ptr<ev_timer>(p_watcher)));
            if (!result.second) {
                cancel_timer(result.first->first);
                result = _timer_watchers.insert(std::make_pair(context, std::unique_ptr<ev_timer>(p_watcher)));
            }
            if (result.second) {
                result.first->second.get()->data = (void*)(&(result.first->first));
                return true;
            }
        }
        return false;
    }

    /* Cancels an event watchig extablished with method `setup_timer` for a given `context`. */
    bool cancel_timer(Context context) {
        auto found = _timer_watchers.find(context);
        if (found != _timer_watchers.end()) {
            auto p_watcher = found->second.get();
            if (ev_is_active(p_watcher))
                ev_timer_stop(_p_loop, p_watcher);
            _timer_watchers.erase(found);
            return true;
        }
        return false;
    }

    /* Setup to call `on_event` for a given `context` 
     * when the system signal with a given `signum` recieved. */
    bool setup_signal(Context context, int signum) {
        auto p_watcher = new ev_signal();
        ev_signal_init(p_watcher, EventLoop::template callback<ev_signal>, signum);
        ev_signal_start(_p_loop, p_watcher);
        if (ev_is_active(p_watcher)) {
            auto result = _signal_watchers.insert(std::make_pair(context, std::unique_ptr<ev_signal>(p_watcher)));
            if (!result.second) {
                cancel_signal(result.first->first);
                result = _signal_watchers.insert(std::make_pair(context, std::unique_ptr<ev_signal>(p_watcher)));
            }
            if (result.second) {
                result.first->second.get()->data = (void*)(&(result.first->first));
                return true;
            }
        }
        return false;
    }    

    /* Cancels an event watchig extablished with method `setup_signal` for a given `context`. */
    bool cancel_signal(Context context) {
        auto found = _signal_watchers.find(context);
        if (found != _signal_watchers.end()) {
            auto p_watcher = found->second.get();
            if (ev_is_active(p_watcher))
                ev_signal_stop(_p_loop, p_watcher);
            _signal_watchers.erase(found);
            return true;
        }
        return false;
    }

};

} // end of base
} // end of squall
#endif
