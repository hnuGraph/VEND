//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_TIMER_H
#define VEND_TIMER_H

#include <chrono>
#include "assert.h"

class Timer {

public:

    Timer() : status(TimerStatus::off), time_cost_ms_(0) {}

    enum class TimerStatus {
        on, off
    };

    void StartTimer() {
        assert(status == TimerStatus::off);
        status = TimerStatus::on;
        time_start_ = std::chrono::steady_clock::now();

    };


    void StopTimer() {
        assert(status == TimerStatus::on);
        std::chrono::steady_clock::time_point time_end = std::chrono::steady_clock::now();
        status = TimerStatus::off;
        time_cost_ms_ += std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start_);
    };


    double CountTime() {
//        return (double)time_cost_ms_.count()*std::chrono::milliseconds::period::num/std::chrono::milliseconds::period::den;
        return (double)time_cost_ms_.count();
//        return time_cost_ms_.count();
    }
    void ResetTime(){
        time_cost_ms_=0*std::chrono::microseconds(1);
    }

private:
    TimerStatus status;
    std::chrono::steady_clock::time_point time_start_;
    std::chrono::microseconds time_cost_ms_;
};


#endif //VEND_TIMER_H
