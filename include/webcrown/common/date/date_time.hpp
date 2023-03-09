#pragma once

#include "date/date.h"

namespace webcrown {

class date_time
{
    date::year_month_day ymd_;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> timepoint_;
public:
    date_time()
    {
        auto tp = std::chrono::system_clock::now();
        auto dp = date::floor<date::days>(tp);
        ymd_ = date::year_month_day{dp};
        timepoint_ = tp;
        // = date::make_time(tp - dp);
    }

    date_time(date::year_month_day ymd, decltype(timepoint_) time)
        : ymd_(ymd)
        , timepoint_(time)
    {}

    date::year_month_day ymd() const noexcept { return ymd_; }
    decltype(timepoint_) time() const noexcept { return timepoint_; }

    std::string str() const
    {
        std::ostringstream os;

        auto time = date::make_time(timepoint_ - date::sys_days(ymd_));

        auto d = date::sys_time<decltype(time.to_duration())>(time.to_duration());

        os 
            << date::format("%Y-%m-%d ", ymd_)
            << date::format("%X", d);

        return os.str();
    }

    u_int32_t days_since(date_time const& rhs)
    {
        auto diff = this->timepoint_ - rhs.timepoint_;
        auto&& result = std::chrono::duration_cast<date::days>(diff).count();

        return result;
    }

    static date_time from_str(std::string const& s)
    {
        date::sys_time<std::chrono::nanoseconds> tp;

        std::istringstream is{s};
        is >> date::parse("%Y-%m-%d %X", tp);
        auto dp = date::floor<date::days>(tp);
        date::year_month_day ymd = date::year_month_day{dp};
        //date::hh_mm_ss<std::chrono::nanoseconds> time = date::make_time(tp - dp);

        return date_time{ymd, tp};
    }

    date_time& operator-(date_time const& rhs)
    {
        auto ymd_r = std::chrono::duration_cast<date::days>(date::sys_days(this->ymd_) - date::sys_days(rhs.ymd_));

        auto diff = this->timepoint_ - rhs.timepoint_;
        auto tp = date::sys_time<std::chrono::nanoseconds>(date::sys_days(this->ymd_).time_since_epoch() - diff);
        auto dp = date::floor<date::days>(tp);
        auto ymd = date::year_month_day{dp};

        this->ymd_ = ymd;
        this->timepoint_ = tp;

        return *this;
    }

};

inline std::ostream& operator<<(std::ostream& os, date_time const dt)
{
    return os << dt.str();
}

}