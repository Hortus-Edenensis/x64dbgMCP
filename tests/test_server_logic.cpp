#include "ServerLogic.h"

#include <iostream>
#include <string>

static int g_failures = 0;

#define EXPECT_TRUE(cond) do { \
    if (!(cond)) { \
        std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__ << " EXPECT_TRUE(" #cond ")\n"; \
        ++g_failures; \
    } \
} while (0)

#define EXPECT_EQ(a, b) do { \
    auto _a = (a); \
    auto _b = (b); \
    if (!((_a) == (_b))) { \
        std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__ << " EXPECT_EQ(" << _a << ", " << _b << ")\n"; \
        ++g_failures; \
    } \
} while (0)

static void test_url_decode() {
    EXPECT_EQ(UrlDecode("hello%20world"), std::string("hello world"));
    EXPECT_EQ(UrlDecode("a+b%2Bc"), std::string("a b+c"));
}

static void test_parse_query_params() {
    auto params = ParseQueryParams("a=1&b=hello%20world&c=%2Btest");
    EXPECT_EQ(params["a"], std::string("1"));
    EXPECT_EQ(params["b"], std::string("hello world"));
    EXPECT_EQ(params["c"], std::string("+test"));
}

static void test_parse_bool() {
    EXPECT_TRUE(ParseBool("true", false));
    EXPECT_TRUE(ParseBool("1", false));
    EXPECT_TRUE(!ParseBool("false", true));
    EXPECT_TRUE(!ParseBool("0", true));
    EXPECT_TRUE(ParseBool("", true));
}

static void test_sanitize_pattern() {
    EXPECT_EQ(SanitizePattern("48 8B 05 ? ?"), std::string("488B05??"));
}

static void test_should_resume_after_pause() {
    EXPECT_TRUE(ShouldResumeAfterPause(true, true, false, true));
    EXPECT_TRUE(!ShouldResumeAfterPause(true, false, false, true));
    EXPECT_TRUE(!ShouldResumeAfterPause(true, true, true, true));
    EXPECT_TRUE(!ShouldResumeAfterPause(true, true, false, false));
    EXPECT_TRUE(!ShouldResumeAfterPause(false, true, false, true));
}

int main() {
    test_url_decode();
    test_parse_query_params();
    test_parse_bool();
    test_sanitize_pattern();
    test_should_resume_after_pause();

    if (g_failures == 0) {
        std::cout << "All tests passed.\n";
        return 0;
    }

    std::cerr << g_failures << " tests failed.\n";
    return 1;
}
