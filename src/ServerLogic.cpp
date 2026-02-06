#include "ServerLogic.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_set>
#include <vector>

static std::string ToLowerAscii(const std::string& value) {
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower;
}

static std::vector<std::string> TokenizeCommand(const std::string& cmd) {
    std::string trimmed = cmd;
    trimmed.erase(trimmed.begin(),
                  std::find_if(trimmed.begin(), trimmed.end(),
                               [](unsigned char c) { return !std::isspace(c); }));
    std::istringstream iss(trimmed);
    std::vector<std::string> tokens;
    for (std::string token; iss >> token;) {
        tokens.push_back(ToLowerAscii(token));
    }
    return tokens;
}

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
    std::string lower = ToLowerAscii(value);
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

bool ParseUnsignedMaybeHex(const std::string& text, unsigned long long& valueOut) {
    if (text.empty()) {
        return false;
    }
    int base = 10;
    if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        base = 16;
    } else {
        for (unsigned char c : text) {
            if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                base = 16;
                break;
            }
        }
    }
    try {
        size_t idx = 0;
        valueOut = std::stoull(text, &idx, base);
        return idx == text.size();
    } catch (...) {
        return false;
    }
}

bool ParseAddressWithMax(const std::string& text, unsigned long long maxValue, unsigned long long& valueOut) {
    if (!ParseUnsignedMaybeHex(text, valueOut)) {
        return false;
    }
    if (valueOut > maxValue) {
        return false;
    }
    return true;
}

bool SanitizeHexBytes(const std::string& input, std::string& output) {
    output.clear();
    output.reserve(input.size());
    bool sawPrefix = false;
    for (size_t i = 0; i < input.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(input[i]);
        if (std::isspace(c)) {
            continue;
        }
        if (!sawPrefix && output.empty() && c == '0' && i + 1 < input.size() &&
            (input[i + 1] == 'x' || input[i + 1] == 'X')) {
            sawPrefix = true;
            ++i;
            continue;
        }
        if (!std::isxdigit(c)) {
            return false;
        }
        output.push_back(static_cast<char>(c));
    }
    if (output.empty()) {
        return false;
    }
    return (output.size() % 2) == 0;
}

CommandRisk ClassifyExecCommandRisk(const std::string& cmd) {
    static const std::unordered_set<std::string> kReadOnly = {
        "help", "?", "eip", "rip", "cip", "csp", "cax", "cbp",
        "rtr", "script", "sc", "getcmdline", "getthreadid",
        "mod", "modinfo", "modbase", "modlist", "modulelist",
        "sym", "symfind", "symaddr", "symname",
        "eval", "print", "printf", "dump", "db", "dw", "dd", "dq",
        "disasm", "dis", "disasmaddr",
        "ref", "refget", "reffind",
        "memlist", "memmap", "meminfo", "memdump",
        "commentlist", "labelist", "bookmarklist",
        "stack", "threads", "threadlist",
        "bplist"
    };

    const std::vector<std::string> tokens = TokenizeCommand(cmd);
    if (tokens.empty()) {
        return CommandRisk::Dangerous;
    }

    const std::string& first = tokens[0];
    if (first == "bp") {
        return tokens.size() == 1 ? CommandRisk::WhitelistReadOnly : CommandRisk::Dangerous;
    }
    if (kReadOnly.find(first) != kReadOnly.end()) {
        return CommandRisk::WhitelistReadOnly;
    }
    return CommandRisk::Dangerous;
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
