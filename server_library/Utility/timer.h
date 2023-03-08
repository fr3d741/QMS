#pragma once

#include <chrono>

class Timer
{
    std::chrono::steady_clock::time_point _clock;
    bool _started;
public:
    Timer();

    void ReStart();
    void Stop();

    int ElapsedSeconds() const;

};
