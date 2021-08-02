#pragma once

#include <Windows.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>

#define PARAM_UNINITIALIZED (-1)

#define PARAM_SWITCH1 '/'
#define PARAM_SWITCH2 '-'

namespace paramkit {

    bool strequals(const std::string& a, const std::string& b, bool ignoreCase = true);

    bool GetColor(HANDLE hConsole, int& color);
    void print_in_color(int color, const std::string &text);

    bool is_param(const std::string &str);

    void print_param_in_color(int color, const std::string &text);

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
        if (strequals(str, "True") || strequals(str, "on") || strequals(str, "yes")) {
            return true;
        }
        if (strequals(str, "False") || strequals(str, "off") || strequals(str, "no")) {
            return false;
        }
        const int val = loadInt(str.c_str(), false);
        if (val == 0) return false;
        return true;
    }


};
