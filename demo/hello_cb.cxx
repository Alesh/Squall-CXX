#include <iostream>
#include <squall/EventLoop.hxx>

using squall::Event;
using squall::EventLoop;

int main(int argc, char const* argv[]) {

    int cnt = 0;
    EventLoop event_loop;

    event_loop.setup_timer([](int revents) {
        if (revents == Event::Timeout)
            std::cout << "Hello, Alesh! (" << revents << ")" << std::endl;
        return true;
    }, 1.0);

    event_loop.setup_timer([&cnt](int revents) {
        if (revents == Event::Timeout) {
            std::cout << "Hello, World! (" << revents << ")" << std::endl;
            cnt++;
        }
        return (cnt < 3);
    }, 2.5);

    event_loop.setup_signal([&event_loop](int revents) {
        if (revents == Event::Signal) {
            std::cout << "\nGot SIGINT. Bye! (" << revents << ")" << std::endl;
            event_loop.stop();            
        }
        return false;
    }, SIGINT);

    event_loop.start();
    return 0;
}