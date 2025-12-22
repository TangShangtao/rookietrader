//
// Created by root on 2025/10/23.
//

#pragma once
#include <string>
#include <chrono>
#include <string_view>
namespace rk::util
{
    class DateTime;
    class TimeDelta
    {
    public:
        struct keyword_args
        {
            int days = 0;
            int hours = 0;
            int minutes = 0;
            int seconds = 0;
            int milliseconds = 0;
            int microseconds = 0;
            int nanoseconds = 0;
        };
        explicit TimeDelta(const keyword_args& _keyword_args)
        {
            _duration = std::chrono::days{_keyword_args.days} +
                        std::chrono::hours{_keyword_args.hours} +
                        std::chrono::minutes{_keyword_args.minutes} +
                        std::chrono::seconds{_keyword_args.seconds} +
                        std::chrono::milliseconds{_keyword_args.milliseconds} +
                        std::chrono::microseconds{_keyword_args.microseconds} +
                        std::chrono::nanoseconds{_keyword_args.nanoseconds};
        }
        template<typename Rep, typename Period>
        explicit TimeDelta(std::chrono::duration<Rep, Period> d): _duration(std::chrono::duration_cast<std::chrono::nanoseconds>(d)) {}
        [[nodiscard]] long days() const {return std::chrono::duration_cast<std::chrono::days>(_duration).count();}
        [[nodiscard]] long hours() const {return std::chrono::duration_cast<std::chrono::hours>(_duration).count();}
        [[nodiscard]] long minutes() const {return std::chrono::duration_cast<std::chrono::minutes>(_duration).count();}
        [[nodiscard]] long seconds() const {return std::chrono::duration_cast<std::chrono::seconds>(_duration).count();}
        [[nodiscard]] long milliseconds() const {return std::chrono::duration_cast<std::chrono::milliseconds>(_duration).count();}
        [[nodiscard]] long microseconds() const {return std::chrono::duration_cast<std::chrono::microseconds>(_duration).count();}
        TimeDelta operator+(const TimeDelta& other) const {return TimeDelta(_duration + other._duration);}
        TimeDelta operator-(const TimeDelta& other) const {return TimeDelta(_duration - other._duration);}
        bool operator<(const TimeDelta& other) const {return _duration < other._duration;}
        bool operator==(const TimeDelta& other) const {return _duration == other._duration;}
        [[nodiscard]] std::string to_string() const {
            auto d = std::chrono::duration_cast<std::chrono::days>(_duration);
            auto h = std::chrono::duration_cast<std::chrono::hours>(_duration - d);
            auto m = std::chrono::duration_cast<std::chrono::minutes>(_duration - d - h);
            auto s = std::chrono::duration_cast<std::chrono::seconds>(_duration - d - h - m);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(_duration - d - h - m - s);
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(_duration - d - h - m - s - ms);

            std::stringstream ss;
            if (d.count() != 0) ss << d.count() << " days, ";
            ss << h.count() << ":"
               << std::setfill('0') << std::setw(2) << m.count() << ":"
               << std::setfill('0') << std::setw(2) << s.count();

            if (ms.count() > 0 || us.count() > 0) {
                ss << "." << std::setfill('0') << std::setw(3) << ms.count();
                if (us.count() > 0) {
                    ss << std::setfill('0') << std::setw(3) << us.count();
                }
            }
            return ss.str();
        }
    friend class DateTime;
    private:
        std::chrono::nanoseconds _duration{};
    };
    class DateTime {
    public:
        DateTime() = default;
        explicit DateTime(std::chrono::system_clock::time_point tp) : _time_point(tp)
        {
            auto duration = tp.time_since_epoch();
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
            _sub_second = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);
        }
        explicit DateTime(int64_t nanoseconds) : DateTime(std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::nanoseconds(nanoseconds))))
        {}
        struct keyword_args
        {
            int year = 0;
            int month = 0;
            int day = 0;
            int hour = 0;
            int minute = 0;
            int second = 0;
            int millisecond = 0;
            int microsecond = 0;
            int nanosecond = 0;
        };
        explicit DateTime(const keyword_args& _keyword_args)
        {
            std::tm tm = {};
            tm.tm_year = _keyword_args.year - 1900;
            tm.tm_mon = _keyword_args.month - 1;
            tm.tm_mday = _keyword_args.day;
            tm.tm_hour = _keyword_args.hour;
            tm.tm_min = _keyword_args.minute;
            tm.tm_sec = _keyword_args.second;
            tm.tm_isdst = -1;  // 自动判断夏令时

            std::time_t tt = std::mktime(&tm);
            if (tt == -1) throw std::invalid_argument("Invalid date/time");
            _time_point = std::chrono::system_clock::from_time_t(tt);
            _sub_second = std::chrono::milliseconds{_keyword_args.millisecond} +
                               std::chrono::microseconds{_keyword_args.microsecond} +
                               std::chrono::nanoseconds{_keyword_args.nanosecond};
            _time_point += _sub_second;
        }
        static DateTime now()
        {
            return DateTime(std::chrono::high_resolution_clock::now());
        }
        static DateTime strptime(
            const std::string& datetime_string,
            std::string_view format="%Y-%m-%d %H:%M:%S",
            unsigned int milliseconds = 0,
            unsigned int microseconds = 0,
            unsigned int nanoseconds = 0
        )
        {
            std::tm tm = {};
            std::istringstream ss(datetime_string);
            ss >> std::get_time(&tm, format.data());
            if (ss.fail())
                throw std::invalid_argument(std::format("Failed to parse date string with format: {}", format));
            std::time_t tt = std::mktime(&tm);
            if (tt == -1)
                throw std::invalid_argument("Invalid date/time");
            auto tp = std::chrono::system_clock::from_time_t(tt);

            // 添加亚秒部分
            tp += std::chrono::milliseconds{milliseconds} +
                  std::chrono::microseconds{microseconds} +
                  std::chrono::nanoseconds{nanoseconds};

            return DateTime(tp);
        }
        [[nodiscard]] std::string strftime(std::string_view format="%Y-%m-%d %H:%M:%S.%f") const
        {
            std::time_t tt = std::chrono::system_clock::to_time_t(_time_point);
            std::tm* tm = std::localtime(&tt);

            std::string modified_format{format};

            // 获取亚秒部分
            auto duration = _time_point.time_since_epoch();
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
            auto sub_second = duration - seconds;

            auto total_nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(sub_second).count();
            auto millis = total_nanos / 1000000;
//            auto micros = (total_nanos % 1000000) / 1000;
//            auto nanos = total_nanos % 1000;

            // 替换 %f (微秒，6位)
            size_t f_pos = modified_format.find("%f");
            if (f_pos != std::string::npos) {
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(6) << (total_nanos / 1000);
                modified_format.replace(f_pos, 2, ss.str());
            }

            // 替换 %3 (毫秒)
            size_t ms_pos = modified_format.find("%3");
            if (ms_pos != std::string::npos) {
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(3) << millis;
                modified_format.replace(ms_pos, 2, ss.str());
            }

            // 替换 %6 (微秒)
            size_t us_pos = modified_format.find("%6");
            if (us_pos != std::string::npos) {
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(6) << (total_nanos / 1000);
                modified_format.replace(us_pos, 2, ss.str());
            }

            // 替换 %9 (纳秒)
            size_t ns_pos = modified_format.find("%9");
            if (ns_pos != std::string::npos) {
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(9) << total_nanos;
                modified_format.replace(ns_pos, 2, ss.str());
            }

            char buffer[256];
            if (std::strftime(buffer, sizeof(buffer), modified_format.c_str(), tm) == 0) {
                throw std::runtime_error("Failed to format date");
            }

            return buffer;
        }
        [[nodiscard]] int year() const {
            std::time_t tt = std::chrono::system_clock::to_time_t(_time_point);
            std::tm* tm = std::localtime(&tt);
            return tm->tm_year + 1900;
        }
        [[nodiscard]] int month() const {
            std::time_t tt = std::chrono::system_clock::to_time_t(_time_point);
            std::tm* tm = std::localtime(&tt);
            return tm->tm_mon + 1;
        }
        [[nodiscard]] int day() const {
            std::time_t tt = std::chrono::system_clock::to_time_t(_time_point);
            std::tm* tm = std::localtime(&tt);
            return tm->tm_mday;
        }
        [[nodiscard]] int hour() const {
            std::time_t tt = std::chrono::system_clock::to_time_t(_time_point);
            std::tm* tm = std::localtime(&tt);
            return tm->tm_hour;
        }
        [[nodiscard]] int minute() const {
            std::time_t tt = std::chrono::system_clock::to_time_t(_time_point);
            std::tm* tm = std::localtime(&tt);
            return tm->tm_min;
        }
        [[nodiscard]] int second() const {
            std::time_t tt = std::chrono::system_clock::to_time_t(_time_point);
            std::tm* tm = std::localtime(&tt);
            return tm->tm_sec;
        }
        [[nodiscard]] int weekday() const {
            std::time_t tt = std::chrono::system_clock::to_time_t(_time_point);
            std::tm* tm = std::localtime(&tt);
            return tm->tm_wday;
        }
        [[nodiscard]] int millisecond() const {
            auto duration = _time_point.time_since_epoch();
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration - seconds);
            return millis.count();
        }
        TimeDelta operator-(const DateTime& other) const {return TimeDelta(_time_point - other._time_point);}
        DateTime operator+(const TimeDelta& delta) const {return DateTime(_time_point + delta._duration);}
        DateTime operator-(const TimeDelta& delta) const {return DateTime(_time_point - delta._duration);}
        bool operator<(const DateTime& other) const {return _time_point < other._time_point;}
        bool operator<=(const DateTime& other) const {return _time_point <= other._time_point;}
        bool operator>(const DateTime& other) const {return _time_point > other._time_point;}
        bool operator>=(const DateTime& other) const {return _time_point >= other._time_point;}
        bool operator==(const DateTime& other) const {return _time_point == other._time_point;}
        bool operator!=(const DateTime& other) const {return _time_point != other._time_point;}

    private:
        std::chrono::system_clock::time_point _time_point{};
        std::chrono::nanoseconds _sub_second{};
    };

};