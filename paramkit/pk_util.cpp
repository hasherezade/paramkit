#include "pk_util.h"

bool paramkit::is_hex(const char *buf, size_t len)
{
    if (!buf || len == 0) return false;
    for (size_t i = 0; i < len; i++) {
        if (buf[i] >= '0' && buf[i] <= '9') continue;
        if (buf[i] >= 'A' && buf[i] <= 'F') continue;
        if (buf[i] >= 'a' && buf[i] <= 'f') continue;
        return false;
    }
    return true;
}

bool paramkit::is_dec(const char *buf, size_t len)
{
    if (!buf || len == 0) return false;
    for (size_t i = 0; i < len; i++) {
        if (buf[i] >= '0' && buf[i] <= '9') continue;
        return false;
    }
    return true;
}

bool paramkit::is_hex_with_prefix(const char *my_buf)
{
    if (!my_buf) return false;

    const char hex_pattern[] = "0x";
    size_t hex_pattern_len = strlen(hex_pattern);

    const size_t len = strlen(my_buf);
    if (len == 0) return false;

    if (len > hex_pattern_len) {
        if (is_cstr_equal(my_buf, hex_pattern, hex_pattern_len)) {
            if (!is_hex(my_buf + hex_pattern_len, len - hex_pattern_len)) return false;
            return true;
        }
    }
    return false;
}

bool paramkit::is_number(const char* my_buf)
{
    if (!my_buf) return false;

    const size_t len = strlen(my_buf);
    if (is_hex_with_prefix(my_buf)) return true;
    if (is_dec(my_buf, len)) return true;
    return false;
}

long paramkit::get_number(const char *my_buf)
{
    if (!my_buf) return false;

    const char hex_pattern[] = "0x";
    size_t hex_pattern_len = strlen(hex_pattern);

    const size_t len = strlen(my_buf);
    if (len == 0) return 0;

    long out = 0;
    const size_t min_length = 1; //tolerate number with at least 1 character
    if (len > hex_pattern_len) {
        if (is_cstr_equal(my_buf, hex_pattern, hex_pattern_len)) {
            if (!is_hex(my_buf + hex_pattern_len, min_length)) return 0;

            std::stringstream ss;
            ss << std::hex << my_buf;
            ss >> out;
            return out;
        }
    }
    if (!is_dec(my_buf, min_length)) return 0;

    std::stringstream ss;
    ss << std::dec << my_buf;
    ss >> out;
    return out;
}

bool paramkit::is_cstr_equal(char const *a, char const *b, const size_t max_len, bool ignoreCase)
{
    if (a == b) return true;
    if (!a || !b) return false;
    for (size_t i = 0; i < max_len; ++i) {
        if (ignoreCase) {
            if (tolower(a[i]) != tolower(b[i])) {
                return false;
            }
        }
        else {
            if (a[i] != b[i]) {
                return false;
            }
        }
        if (a[i] == '\0') break;
    }
    return true;
}

bool paramkit::strequals(const std::string& a, const std::string& b, bool ignoreCase)
{
    size_t aLen = a.size();
    if (b.size() != aLen) return false;

    for (size_t i = 0; i < aLen; ++i) {
        if (!ignoreCase) {
            if (a[i] != b[i]) return false;
        }
        else {
            if (tolower(a[i]) != tolower(b[i])) return false;
        }
    }
    return true;
}

bool paramkit::get_console_color(HANDLE hConsole, int& color) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(hConsole, &info))
        return false;
    color = info.wAttributes;
    return true;
}

void paramkit::print_in_color(int color, const std::string &text)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int prev = 7;
    get_console_color(hConsole, prev); // get the previous color

    FlushConsoleInputBuffer(hConsole);
    SetConsoleTextAttribute(hConsole, color); // back to default color
    std::cout << text;
    FlushConsoleInputBuffer(hConsole);

    SetConsoleTextAttribute(hConsole, prev); // back to previous color
}

