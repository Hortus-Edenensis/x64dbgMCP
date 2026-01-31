#ifndef SERVER_LOGIC_H
#define SERVER_LOGIC_H

#include <string>
#include <unordered_map>

std::string UrlDecode(const std::string& str);
std::unordered_map<std::string, std::string> ParseQueryParams(const std::string& query);
bool ParseBool(const std::string& value, bool defaultValue);
std::string SanitizePattern(const std::string& pattern);

// Business logic helpers
bool ShouldResumeAfterPause(bool runUntilActive,
                            bool autoResume,
                            bool userBreakpointHit,
                            bool isSystemModule);

#endif
