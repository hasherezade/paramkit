/**
* @file
* @brief   The set of utility functions used by the ParamKit
*/

#pragma once

#include <windows.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <set>

#include "strings_util.h"

#define GETNAME(x) (#x)

namespace paramkit {

    size_t strip_to_list(IN std::string s, IN std::string delim, OUT std::set<std::string>& elements_list);
    std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ");

    bool get_console_color(HANDLE hConsole, WORD& color);
    void print_in_color(WORD color, const std::string& text, DWORD out_hndl=STD_OUTPUT_HANDLE);

    bool is_hex(const char *buf, size_t len);
    bool is_hex_with_prefix(const char *buf);
    bool is_dec(const char *buf, size_t len);
    bool is_number(const char* my_buf);

    template <typename T_INT>
    T_INT get_number(const char* my_buf)
    {
        if (!my_buf) return T_INT(0);

        const char hex_pattern[] = "0x";
        size_t hex_pattern_len = strlen(hex_pattern);

        const size_t len = strlen(my_buf);
        if (len == 0) return T_INT(0);

        T_INT out = 0;
        if (len > hex_pattern_len) {
            if (util::is_cstr_equal(my_buf, hex_pattern, hex_pattern_len)) {
                if (!is_hex(my_buf + hex_pattern_len, len - hex_pattern_len)) return T_INT(0);

                std::stringstream ss;
                ss << std::hex << my_buf;
                ss >> out;
                return out;
            }
        }
        if (!is_dec(my_buf, len)) return T_INT(0);

        std::stringstream ss;
        ss << std::dec << my_buf;
        ss >> out;
        return out;
    }

    template <typename T_CHAR>
    std::string to_string(T_CHAR *str1)
    {
        if (str1 == nullptr) return "";
        std::string val;

        for (size_t i = 0; ; i++) {
            if (str1[i] == 0) break;
            val.push_back((char)str1[i]);
        }
        return val;
    }

    // Specialisation for wide strings: uses WideCharToMultiByte (CP_UTF8)
    // so that non-ASCII characters are preserved rather than silently truncated.
    template <>
    inline std::string to_string<wchar_t>(wchar_t *str1)
    {
        if (str1 == nullptr) return "";
        const int size = WideCharToMultiByte(CP_UTF8, 0, str1, -1, nullptr, 0, nullptr, nullptr);
        if (size <= 0) return "";
        std::string val(static_cast<size_t>(size) - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, str1, -1, &val[0], size, nullptr, nullptr);
        return val;
    }

    template <typename T_INT, typename T_CHAR>
    T_INT loadInt(const T_CHAR *str1, bool isHex = false)
    {
        std::string str = to_string(str1);
        T_INT intVal = 0;
        std::stringstream ss;
        if (isHex) {
            ss << std::hex << str;
        }
        else {
            ss << std::dec << str;
        }
        ss >> intVal;
        return intVal;
    }

    template <typename T_CHAR>
    bool loadBoolean(IN const T_CHAR *str1, OUT bool &value)
    {
        std::string str = to_string(str1);
        if (util::strequals(str, "True") || util::strequals(str, "on") || util::strequals(str, "yes")) {
            value = true;
            return true;
        }
        if (util::strequals(str, "False") || util::strequals(str, "off") || util::strequals(str, "no")) {
            value = false;
            return true;
        }
        if (!is_dec(str.c_str(), str.length())) {
            return false;
        }
        const int val = loadInt<int>(str.c_str(), false);
        if (val == 0) {
            value = false;
            return true;
        }
        if (val == 1) {
            value = true;
            return true;
        }
        return false;
    }

    //! Copy the std::string/std::wstring value into an buffer of a given character count
    template <typename T_CHAR>
    size_t copy_to_cstr(const std::basic_string<T_CHAR>& value, T_CHAR* buf, size_t buf_count)
    {
        if (!buf || buf_count == 0) {
            return 0;
        }

        const size_t chars_to_copy = (value.length() < (buf_count - 1)) ? value.length() : (buf_count - 1);

        if (chars_to_copy > 0) {
            ::memcpy(buf, value.c_str(), chars_to_copy * sizeof(T_CHAR));
        }
        buf[chars_to_copy] = T_CHAR{};
        return chars_to_copy;
    }

};
