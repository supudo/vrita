#ifndef VRITA_DATETIMES_INCLUDES
#define VRITA_DATETIMES_INCLUDES

#include <chrono>
#include <filesystem>
#include <format>

namespace VritaUtils {
    static inline std::string getDateToStringFormatted(const std::filesystem::file_time_type& fileTime, const std::string& dateFormat) {
        using namespace std::chrono;
#if defined(_MSC_VER)
        return std::format("{:%F %T}", clock_cast<system_clock>(fileTime));
#else
        return std::format("{:%F %T}", file_clock::to_sys(fileTime));
#endif
    }
}

#endif