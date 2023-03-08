#include <Utility/timer.h>

Timer::Timer() {
    ReStart();
}

void
Timer::ReStart(){
    _clock = std::chrono::steady_clock::now();
    _started = true;
}

void
Timer::Stop(){
    _started = false;
}

int
Timer::ElapsedSeconds() const {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - _clock).count();
}
