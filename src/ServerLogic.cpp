#include "ServerLogic.h"

#include <algorithm>
#include <cctype>
#include <sstream>

std::string UrlDecode(const std::string& str) {
    std::string decoded;
    decoded.reserve(str.size());
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value = 0;
            std::istringstream is(str.substr(i + 1, 2));
            if (is >> std::hex >> value) {
                decoded += static_cast<char>(value);
                i += 2;
            } else {
                decoded += str[i];
            }
        } else if (str[i] == '+') {
            decoded += ' ';
        } else {
            decoded += str[i];
        }
    }
    return decoded;
}

std::unordered_map<std::string, std::string> ParseQueryParams(const std::string& query) {
    std::unordered_map<std::string, std::string> params;

    size_t pos = 0;
    size_t nextPos;

    while (pos < query.length()) {
        nextPos = query.find('&', pos);
        if (nextPos == std::string::npos) {
            nextPos = query.length();
        }

        std::string pair = query.substr(pos, nextPos - pos);
        size_t equalPos = pair.find('=');

        if (equalPos != std::string::npos) {
            std::string key = pair.substr(0, equalPos);
            std::string value = pair.substr(equalPos + 1);
            params[UrlDecode(key)] = UrlDecode(value);
        }

        pos = nextPos + 1;
    }

    return params;
}

bool ParseBool(const std::string& value, bool defaultValue) {
    if (value.empty()) {
        return defaultValue;
    }
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "1" || lower == "true" || lower == "yes" || lower == "on") {
        return true;
    }
    if (lower == "0" || lower == "false" || lower == "no" || lower == "off") {
        return false;
    }
    return defaultValue;
}

std::string SanitizePattern(const std::string& pattern) {
    std::string sanitized = pattern;
    sanitized.erase(std::remove_if(sanitized.begin(), sanitized.end(),
                                   [](unsigned char c) { return std::isspace(c); }),
                    sanitized.end());
    return sanitized;
}

bool ShouldResumeAfterPause(bool runUntilActive,
                            bool autoResume,
                            bool userBreakpointHit,
                            bool isSystemModule) {
    if (!runUntilActive) {
        return false;
    }
    if (!autoResume) {
        return false;
    }
    if (userBreakpointHit) {
        return false;
    }
    return isSystemModule;
}
