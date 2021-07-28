#include "pk_util.h"

bool paramkit::GetColor(HANDLE hConsole, int& color) {
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
    GetColor(hConsole, prev); // get the previous color

    FlushConsoleInputBuffer(hConsole);
    SetConsoleTextAttribute(hConsole, color); // back to default color
    std::cout << text;
    FlushConsoleInputBuffer(hConsole);

    SetConsoleTextAttribute(hConsole, prev); // back to previous color
}

std::string paramkit::to_string(const char *str)
{
    return std::string(str);
}

std::string paramkit::to_string(const wchar_t *cstr)
{
    std::wstring wstr = wstr;
    std::string str(wstr.begin(), wstr.end());
    return std::string(str);
}

int paramkit::loadInt(const std::string &str, bool isHex)
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

int paramkit::loadInt(const std::wstring &wstr, bool isHex)
{
    int intVal = 0;
    std::string str(wstr.begin(), wstr.end());
    return loadInt(str, isHex);
}

bool paramkit::is_param(const std::string str)
{
    if (str.length() == 0) return false;

    const size_t len = str.length();
    if (len < 2) return false;

    if (str[0] == PARAM_SWITCH1 || str[0] == PARAM_SWITCH2) {
        return true;
    }
    return false;
}
