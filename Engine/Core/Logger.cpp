#include "Logger.h"

#include <cctype>
#include <iostream>

namespace SAGE {

    void Logger::Init() {
        std::cout << "[LOGGER] SAGE Engine logger initialized" << std::endl;
    }

    void Logger::Log(LogLevel level, const std::string& message) {
        std::string prefix;
        switch (level) {
            case LogLevel::Trace:   prefix = "[TRACE] "; break;
            case LogLevel::Info:    prefix = "[INFO] "; break;
            case LogLevel::Warning: prefix = "[WARNING] "; break;
            case LogLevel::Error:   prefix = "[ERROR] "; break;
        }

        if (level == LogLevel::Error)
            std::cerr << prefix << message << std::endl;
        else
            std::cout << prefix << message << std::endl;
    }

    std::string Logger::Format(const std::string& format, const std::vector<std::string>& args) {
        std::string result;
        result.reserve(format.size() + args.size() * 4);

        size_t sequentialIndex = 0;

        for (size_t i = 0; i < format.size(); ++i) {
            char c = format[i];
            if (c == '{') {
                size_t j = i + 1;
                bool hasIndex = false;
                size_t explicitIndex = 0;

                while (j < format.size() && std::isdigit(static_cast<unsigned char>(format[j]))) {
                    hasIndex = true;
                    explicitIndex = explicitIndex * 10 + static_cast<size_t>(format[j] - '0');
                    ++j;
                }

                if (j < format.size() && format[j] == '}') {
                    size_t indexToUse = hasIndex ? explicitIndex : sequentialIndex++;
                    if (indexToUse < args.size()) {
                        result += args[indexToUse];
                    } else {
                        result += "{}";
                    }
                    i = j; // skip until '}'
                    continue;
                }
            }

            if (c == '}' && i + 1 < format.size() && format[i + 1] == '}') {
                result += '}';
                ++i;
                continue;
            }

            result += c;
        }

        return result;
    }

}
