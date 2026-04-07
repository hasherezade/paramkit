#include "pk_util.h"
#include "strings_util.h"

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
        if (util::is_cstr_equal(my_buf, hex_pattern, hex_pattern_len)) {
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

bool paramkit::get_console_color(HANDLE hConsole, WORD& color)
{
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (!GetConsoleScreenBufferInfo(hConsole, &info))
        return false;
    color = info.wAttributes;
    return true;
}

void paramkit::print_in_color(WORD color, const std::string &text, DWORD out_hndl)
{
    HANDLE hConsole = GetStdHandle(out_hndl);
    if (hConsole == INVALID_HANDLE_VALUE || hConsole == NULL) {
        std::cout << text;
        return;
    }
    const bool is_error = (out_hndl == STD_ERROR_HANDLE) ? true : false;
    std::ostream& stream = is_error ? std::cerr : std::cout;
    WORD prev = 7;
    const bool is_retrieved = get_console_color(hConsole, prev);
    if (is_retrieved) {
        stream << std::flush;
        SetConsoleTextAttribute(hConsole, color);
    }

    stream << text << std::flush;

    if (is_retrieved) {
        SetConsoleTextAttribute(hConsole, prev); // back to previous color
    }
}

namespace paramkit {
    std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
    {
        str.erase(0, str.find_first_not_of(chars));
        return str;
    }

    std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
    {
        str.erase(str.find_last_not_of(chars) + 1);
        return str;
    }
};

std::string& paramkit::trim(std::string& str, const std::string& chars)
{
    return ltrim(rtrim(str, chars), chars);
}

size_t paramkit::strip_to_list(IN std::string s, IN std::string delim, OUT std::set<std::string> &elements_list)
{
    size_t start = 0;
    size_t end = s.find(delim);
    while (end != std::string::npos)
    {
        std::string next_str = s.substr(start, end - start);
        trim(next_str);
        if (next_str.length() > 0) {
            elements_list.insert(next_str);
        }
        start = end + delim.length();
        end = s.find(delim, start);
    }
    std::string next_str = s.substr(start, end);
    trim(next_str);
    if (next_str.length() > 0) {
        elements_list.insert(next_str);
    }
    return elements_list.size();
}
