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

    bool is_hex(const char *buf, size_t len);
    bool is_hex_with_prefix(const char *buf);
    bool is_dec(const char *buf, size_t len);
    bool is_number(const char* my_buf);
    long get_number(const char *my_buf);

    size_t strip_to_list(IN std::string s, IN std::string delim, OUT std::set<std::string> &elements_list);
    std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ");

    bool get_console_color(HANDLE hConsole, int& color);
    void print_in_color(int color, const std::string &text);
    //--

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

    template <typename T_CHAR>
    int loadInt(const T_CHAR *str1, bool isHex = false)
    {
        std::string str = to_string(str1);
        int intVal = 0;
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
        const int val = loadInt(str.c_str(), false);
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
    template <typename T_STR, typename T_CHAR>
    size_t copy_to_cstr(T_STR value, T_CHAR *buf, size_t buf_count)
    {
        size_t val_len = value.length() + 1;
        if (val_len > buf_count) {
            val_len = buf_count;
        }
        memcpy(buf, value.c_str(), val_len * sizeof(T_CHAR));
        buf[val_len - 1] = '\0';
        return val_len;
    }

};
