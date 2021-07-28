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

void paramkit::print_param_in_color(int color, const std::string &text)
{
    print_in_color(color, PARAM_SWITCH1 + text);
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
