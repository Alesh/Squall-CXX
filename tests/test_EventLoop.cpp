#include "../include/squall/base/EventLoop.hxx"
#include "../include/squall/EventLoop.hxx"
#include "catch.hpp"


TEST_CASE("squall::base::EventLoop<const char*>", "[EventLoop]") {

    using squall::Event;
    using EventLoop = squall::base::EventLoop<const char*>;

    int cnt = 0;
    std::string result;
    const char* A = "A";
    const char* B = "B";
    const char* W = "W";

    EventLoop event_loop([&cnt, &result, &B, &W, &event_loop](const char* Ch, int revents) {
        result += Ch;
        if (Ch[0] == 'A') {
            cnt++;
            if (cnt==1)
                event_loop.setup_io(W, 0, Event::Write);
            else if (cnt==6)
                event_loop.cancel_timer(B);
            else if (cnt==10)
                event_loop.stop();
        }
        else if (Ch[0] == 'W')
            event_loop.cancel_io(W);
    });

    event_loop.setup_timer(A, 0.1);
    event_loop.setup_timer(B, 0.26);
    event_loop.start();

    REQUIRE(result == "AWABAAABAAAAA");
}


TEST_CASE("squall::EventLoop", "[EventLoop]") {
    using squall::Event;
    using squall::EventLoop;

    int A = 0;
    int B = 0;
    std::string result;
    EventLoop event_loop;

    event_loop.setup_timer([&](int revents){
        A++;
        result += 'A';
        if (A == 1) {
            event_loop.setup_io([&](int revents){
                result += 'W';
                return false;
            }, 0, Event::Write);
        }
        else if (A == 10)
            event_loop.stop();
        return true;
    }, 0.1);    

    event_loop.setup_timer([&](int revents){
        B = B + 1;
        result += "B";
        return (B < 2);
    }, 0.26);

    event_loop.start();
    REQUIRE(result == "AWABAAABAAAAA");    
}