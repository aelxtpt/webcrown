#pragma once
#include <chrono>
#include <ctime>
#include <string>

namespace webcrown {

typedef int64_t Nanos;

constexpr Nanos NANOS_TO_MICROS = 1000;
constexpr Nanos MICROS_TO_MILLIS = 1000;
constexpr Nanos MILLIS_TO_SECS = 1000;
constexpr Nanos NANOS_TO_MILLIS = NANOS_TO_MICROS * MICROS_TO_MILLIS;
constexpr Nanos NANOS_TO_SECS = NANOS_TO_MILLIS * MILLIS_TO_SECS;

inline
auto current_nanos() noexcept {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

inline
auto& current_time_str(std::string* time_str) {
    auto const time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        time_str->assign(ctime(&time));
    
    if(!time_str->empty())
        time_str->at(time_str->length() -1) = '\0';

    return *time_str;
}

}