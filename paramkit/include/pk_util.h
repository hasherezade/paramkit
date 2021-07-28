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

    bool GetColor(HANDLE hConsole, int& color) {
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (!GetConsoleScreenBufferInfo(hConsole, &info))
            return false;
        color = info.wAttributes;
        return true;
    }

    void print_in_color(int color, const std::string &text)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        int prev = 7;
        GetColor(hConsole, prev); // get the previous color

        FlushConsoleInputBuffer(hConsole);
        SetConsoleTextAttribute(hConsole, color); // back to default color
        std::cout << text;
        FlushConsoleInputBuffer(hConsole);

        SetConsoleTextAttribute(hConsole, prev); // back to previous color
    }

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

    template <typename T_CHAR>
    bool is_param(const T_CHAR *str)
    {
        if (!str) return false;

        const size_t len = str_len(str);
        if (len < 2) return false;

        if (str[0] == PARAM_SWITCH1 || str[0] == PARAM_SWITCH2) {
            return true;
        }
        return false;
    }

    //--

    std::string to_string(const char *str)
    {
        return std::string(str);
    }

    std::string to_string(const wchar_t *cstr)
    {
        std::wstring wstr = wstr;
        std::string str(wstr.begin(), wstr.end());
        return std::string(str);
    }

    static int loadInt(const std::string &str, bool isHex = false)
    {
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

    static int loadInt(const std::wstring &wstr, bool isHex = false)
    {
        int intVal = 0;
        std::string str(wstr.begin(), wstr.end());
        return loadInt(str, isHex);
    }
};
