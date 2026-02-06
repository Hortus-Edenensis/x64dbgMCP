#ifndef SERVER_LOGIC_H
#define SERVER_LOGIC_H

#include <string>
#include <unordered_map>
#include <cstdint>

std::string UrlDecode(const std::string& str);
std::unordered_map<std::string, std::string> ParseQueryParams(const std::string& query);
bool ParseBool(const std::string& value, bool defaultValue);
std::string SanitizePattern(const std::string& pattern);

enum class CommandRisk {
    WhitelistReadOnly,
    Dangerous
};

bool ParseUnsignedMaybeHex(const std::string& text, unsigned long long& valueOut);
bool ParseAddressWithMax(const std::string& text, unsigned long long maxValue, unsigned long long& valueOut);
bool SanitizeHexBytes(const std::string& input, std::string& output);
CommandRisk ClassifyExecCommandRisk(const std::string& cmd);

// Business logic helpers
bool ShouldResumeAfterPause(bool runUntilActive,
                            bool autoResume,
                            bool userBreakpointHit,
                            bool isSystemModule);

#endif
