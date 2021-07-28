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

    bool GetColor(HANDLE hConsole, int& color);
    void print_in_color(int color, const std::string &text);

    template <typename T_CHAR>
    bool str_len(T_CHAR *str1)
    {
        for (size_t i = 0; ; i++) {
            if (str1[i] == '\0') {
                return i;
            }
        }
        return 0;
    }

    bool is_param(const std::string str);
    //--

    template <typename T_CHAR>
    std::string to_string(T_CHAR *str1)
    {
        if (str1 == nullptr) return "";
        std::string val;

        for (size_t i = 0; ; i++) {
            if (str1[i] == 0) break;
            val.push_back(str1[i]);
        }
        return val;
    }

    int loadInt(const std::string &str, bool isHex = false);

    int loadInt(const std::wstring &wstr, bool isHex = false);
};
