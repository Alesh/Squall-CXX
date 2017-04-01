#include <iostream>
#include <squall/base/EventLoop.hxx>


using squall::Event;
using EventLoop = squall::base::EventLoop<const char*>;


int main(int argc, char const* argv[]) {
    EventLoop event_loop([&event_loop](const char* name, int revents) {
        if (revents == Event::Timeout) {
            std::cout << "Hello, " << name << "! (" << revents << ")" << std::endl;
        } else 
        if (revents == Event::Signal) {
            std::cout << "\nGot " << name << ". Bye! (" << revents << ")" << std::endl;
            event_loop.stop();            
        }
    });

    event_loop.setup_timer("Alesh", 1.0);
    event_loop.setup_timer("World", 2.5);
    event_loop.setup_signal("SIGINT", SIGINT);
    event_loop.start();
    return 0;
}