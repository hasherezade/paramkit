/**
* @file
* @brief   The set of utility functions used by the ParamKit
*/

#pragma once

#include <Windows.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>

#define GETNAME(x) (#x)

namespace paramkit {

    bool is_hex(const char *buf, size_t len);
    bool is_hex_with_prefix(const char *buf);
    bool is_dec(const char *buf, size_t len);
    bool is_number(const char* my_buf);
    long get_number(const char *my_buf);

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
    bool loadBoolean(const T_CHAR *str1)
    {
        std::string str = to_string(str1);
        if (util::strequals(str, "True") || util::strequals(str, "on") || util::strequals(str, "yes")) {
            return true;
        }
        if (util::strequals(str, "False") || util::strequals(str, "off") || util::strequals(str, "no")) {
            return false;
        }
        const int val = loadInt(str.c_str(), false);
        if (val == 0) return false;
        return true;
    }

};
