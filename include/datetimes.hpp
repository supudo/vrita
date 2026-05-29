#ifndef VRITA_DATETIMES_INCLUDES
#define VRITA_DATETIMES_INCLUDES

#include <chrono>
#include <format>

namespace VritaUtils {
    static inline std::string getDateToStringFormatted(
        const std::chrono::system_clock::duration& duration,
        const std::string& dateFormat) {
        using namespace std::chrono;
        system_clock::time_point tp = system_clock::time_point{} + duration;
        return std::format("{:%F %T}", tp);
    }
}

#endif